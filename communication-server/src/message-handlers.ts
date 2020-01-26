import WebSocket from 'ws';
import winston from 'winston';

import {
    InboundMessage,
    InboundMessageTypes,
    
    ConnectPIPayload,
    AuthorizeClientPayload,
    CreatePIUserPayload,
    RemovePIUserPayload,
    PIErrorPayload,

    User,
    RegisteredWebClient,
    RegisteredPIInstance,
} from './types';

import {
    notifyBadProtocol,
    notifyBadAuthorization,
    sendPIDevices,
    sendPIUsers,
    sendCreatePIUser,
    sendRemovePIUser,
} from './message-responses';

export interface Storage {
    registeredPIInstances: Map<WebSocket, RegisteredPIInstance>;
    registeredWebClients: Map<WebSocket, RegisteredWebClient>;
}

const storage: Storage = {
    registeredPIInstances: new Map(),
    registeredWebClients: new Map(),
};

new WeakMap()

const IDGenerator = (() => {
    let id = 0;
    return (): number => {
        return ++id;
    };
})();

function userIsValid(user: User): boolean {
    return typeof (user.firstname) === 'string'
        && typeof (user.secondname) === 'string'
        && typeof (user.passport) === 'string'
        && ['string', 'undefined'].includes(typeof (user.patronymic))
        && ['string', 'undefined'].includes(typeof (user.image))
        && ['number', 'undefined'].includes(typeof (user.id));
}

function authentificate(
    socket: WebSocket, 
    registered: Map<WebSocket, RegisteredPIInstance | RegisteredWebClient>,
): RegisteredPIInstance | RegisteredWebClient | undefined {
    return registered.get(socket);
}

function authentificateWebClient(
    socket: WebSocket,
): RegisteredWebClient | undefined {
    return authentificate(socket, storage.registeredWebClients) as RegisteredWebClient | undefined;
}

function authentificatePIDevice(
    socket: WebSocket,
): RegisteredPIInstance | undefined {
    return authentificate(socket, storage.registeredPIInstances) as RegisteredPIInstance | undefined;
}

function authenificatedForPIDevice(
    socket: WebSocket,
    deviceID: number,
): boolean {
    const registeredWebClient: RegisteredWebClient | undefined = storage
        .registeredWebClients.get(socket);
    if (!registeredWebClient) {
        return false;
    }

    return registeredWebClient.isAuthorizedFor.includes(deviceID);
}

function handlePIDeviceConnection(PIDeviceSocket: WebSocket, body: InboundMessage<ConnectPIPayload>): void {
    function notifyWebClients() {
        for (const webClient of storage.registeredWebClients.values()) {
            winston.silly(`Web client ${webClient.id} is being notified about current PI devices`);    
            sendPIDevices(webClient.socket, Array.from(storage.registeredPIInstances.values()));
        }
    }

    try {
        const {
            pin,
            name,
            users,
        } = body?.payload;
        
        winston.info(`PI device is trying to register`);
        if (authentificatePIDevice(PIDeviceSocket)) {
            notifyBadProtocol(PIDeviceSocket);
            throw new Error('This socket has already been registered');
        }

        const valid = typeof(name) === 'string'
            && typeof(pin) === 'number' && Array.isArray(users)
            && users.every((user: User): boolean => userIsValid(user));
        if (!valid) {
            notifyBadProtocol(PIDeviceSocket);
            throw new Error('Bad protocol. Data checking is failed.');
        }

        const generatedID = IDGenerator();
        winston.info(`Registered PI device ${name}. Generated ID: ${generatedID}`);
        storage.registeredPIInstances.set(PIDeviceSocket, {
            id: generatedID,
            pin,
            name,
            users,
            socket: PIDeviceSocket,
        });

        PIDeviceSocket.once('close', () => {
            winston.info(`Connection closed with the PI device: ${name}, id ${generatedID}. Registration removed`);
            const lengthBefore = storage.registeredPIInstances.size;
            storage.registeredPIInstances.delete(PIDeviceSocket);
            const lengthAfter = storage.registeredPIInstances.size;
            winston.debug(`Number of registered PI devices changed from ${lengthBefore} to ${lengthAfter}`);

            notifyWebClients();            
        });

        notifyWebClients();
    } catch (error) {   
        winston.error(error.toString());
    }   
}

function handleWebClientConnection(webClientSocket: WebSocket) {
    winston.info(`Web client is trying to register`);
    if (authentificateWebClient(webClientSocket)) {
        notifyBadProtocol(webClientSocket);
        throw new Error('This client has already been registered');
    }

    const generatedID = IDGenerator();
    winston.info(`Registered web client. Generated ID: ${generatedID}`);
    storage.registeredWebClients.set(webClientSocket, {
        id: generatedID,
        socket: webClientSocket,
        isAuthorizedFor: [],
    });

    webClientSocket.once('close', () => {
        winston.info(`Connection closed with the web client ${generatedID}. Registration removed`);
        const lengthBefore = storage.registeredWebClients.size;
        storage.registeredWebClients.delete(webClientSocket);
        const lengthAfter = storage.registeredWebClients.size;
        winston.debug(`Number of registered web clients changed from ${lengthBefore} to ${lengthAfter}`);
    });

    winston.silly(`Web client ${generatedID} is being notified about current PI devices`);  
    sendPIDevices(webClientSocket, Array.from(storage.registeredPIInstances.values()));
}

function handleWebClientAuthorization(webClientSocket: WebSocket, body: InboundMessage<AuthorizeClientPayload>) {
    try {
        winston.info(`Web client is trying to authorize`);
        const webClient = authentificateWebClient(webClientSocket);
        if (!webClient) {
            winston.error('Authorization failed. Client is not registered');
            notifyBadAuthorization(webClientSocket);
            return;
        }

        const {
            deviceID,
            pin,
        } = body?.payload;
        const valid = typeof (deviceID) === 'number' && typeof (pin) === 'number';
        if (!valid) {
            notifyBadProtocol(webClientSocket);
            throw new Error('Bad protocol. Data checking is failed.');
        }

        const [PIDevice] = Array.from(storage.registeredPIInstances.values())
            .filter((instance: RegisteredPIInstance): boolean => (
                instance.id === deviceID
            ));

        if (!PIDevice || PIDevice.pin !== pin) {
            winston.warn(`Access denied to the PI device ${deviceID} for the client ${webClient.id}`);
            notifyBadAuthorization(webClientSocket);
        } else {
            winston.info(`Web client ${webClient.id} was authorized for device ${deviceID}`);
            if (webClient.isAuthorizedFor.includes(PIDevice.id)) {
                webClient.isAuthorizedFor.push(PIDevice.id);
            }

            sendPIUsers(webClientSocket, PIDevice);
        }
    } catch (error) {
        winston.error(error.toString());
    }
}

function handleCreatePIUser(webClientSocket: WebSocket, body: InboundMessage<CreatePIUserPayload>) {
    try {
        winston.info('Web client is trying to create a PI user')
        const webClient = authentificateWebClient(webClientSocket);
        if (!webClient) {
            notifyBadProtocol(webClientSocket);
            throw new Error('This socket has not been registered yet');
        }

        const {
            deviceID,
            user,
        } = body?.payload;
        const valid = typeof (deviceID) === 'number' 
            && userIsValid(user) 
            && typeof(user.image) === 'string';
        if (!valid) {
            notifyBadProtocol(webClientSocket);
            throw new Error('Bad protocol. Data checking is failed.');
        }

        const [PIDevice] = Array.from(storage.registeredPIInstances.values())
            .filter((instance: RegisteredPIInstance): boolean => (
                instance.id === deviceID
            ));
        if (!authenificatedForPIDevice(webClientSocket, deviceID as number) || !PIDevice) {
            winston.warn(`Access denied to the PI device ${deviceID} for the client ${webClient.id}`);
            notifyBadAuthorization(webClientSocket);
        }

        sendCreatePIUser(PIDevice.socket, user);
    } catch (error) {
        winston.error(error.toString());
    }
}

function handleRemovePIUser(webClientSocket: WebSocket, body: InboundMessage<RemovePIUserPayload>) {
    try {
        winston.info('Web client is trying to remove a PI user')
        const webClient = authentificateWebClient(webClientSocket);
        if (!webClient) {
            notifyBadProtocol(webClientSocket);
            throw new Error('This socket has not been registered yet');
        }

        const {
            deviceID,
            userID,
        } = body?.payload;
        const valid = typeof (deviceID) === 'number' && typeof(userID) === 'number';
        if (!valid) {
            notifyBadProtocol(webClientSocket);
            throw new Error('Bad protocol. Data checking is failed.');
        }

        const [PIDevice] = Array.from(storage.registeredPIInstances.values())
            .filter((instance: RegisteredPIInstance): boolean => (
                instance.id === deviceID
            ));
        if (!authenificatedForPIDevice(webClientSocket, deviceID as number) || !PIDevice) {
            winston.warn(`Access denied to the PI device ${deviceID} for the client ${webClient.id}`);
            notifyBadAuthorization(webClientSocket);
        }

        sendRemovePIUser(PIDevice.socket, userID);
    } catch (error) {
        winston.error(error.toString());
    }
}

function handlePIError(socket: WebSocket, body: InboundMessage<PIErrorPayload>) {
    try {
        // check client authentification
        // resend message to pi if okay
    } catch (error) {

    }
}

export default function handleMessage(
    socket: WebSocket,
    message: WebSocket.Data,
) {
    try {
        if (typeof (message) !== 'string') {
            throw new Error('Invalid protocol. Message must be a string');
        }

        const body = JSON.parse(message) as InboundMessage<any>;

        if (typeof (body.type) !== 'string') {
            throw new Error('Invalid protocol. Message must be a string');
        }

        switch (body.type) {
            case InboundMessageTypes.CONNECT_PI: {
                handlePIDeviceConnection(socket, body);
                break;
            }
            case InboundMessageTypes.CONNECT_WEB_CLIENT: {
                handleWebClientConnection(socket);
                break;
            }
            case InboundMessageTypes.AUTHORIZE_CLIENT: {
                handleWebClientAuthorization(socket, body);
                break;
            }
            case InboundMessageTypes.CREATE_PI_USER: {
                handleCreatePIUser(socket, body);
                break;
            }
            case InboundMessageTypes.REMOVE_PI_USER: {
                handleRemovePIUser(socket, body);
                break;
            }
            case InboundMessageTypes.PI_ERROR: {
                handlePIError(socket, body);
                break;
            }
            default: {
                notifyBadProtocol(socket);
                winston.warn(`Unknown inbound message type: ${body.type}`);
            }
        }
    } catch (error) {
        notifyBadProtocol(socket);
        winston.error(error.toString());
    }
}
