import WebSocket, { CONNECTING } from 'ws';
import winston from 'winston';

import {
    InboundMessage,
    InboundMessageTypes,

    ConnectPIPayload,
    AuthorizeClientPayload,
    CreatePIUserPayload,
    RemovePIUserPayload,

    OutboundMessage,
    OutboundMessageTypes,
    OKStatusPayload,
    ErrorStatusPayload,
    UpdatePIUsersPayload,
    UpdatePIDevicesPayload,

    User,
    WebClient,
    PIDevice,
} from './types';

function userIsValid(user: User): boolean {
    return typeof (user) === 'object'
        && typeof (user.firstname) === 'string'
        && typeof (user.secondname) === 'string'
        && typeof (user.passport) === 'string'
        && ['string', 'undefined'].includes(typeof (user.patronymic))
        && ['string', 'undefined'].includes(typeof (user.image))
        && ['number', 'undefined'].includes(typeof (user.id));
}

class InMemoryStorage {
    private webClients: Map<WebSocket, WebClient>;
    private piDevices: Map<WebSocket, PIDevice>;
    private idGenerator: number;

    public constructor() {
        this.webClients = new Map();
        this.piDevices = new Map();
        this.idGenerator = 0;
    }

    public addWebClient(webClientSocket: WebSocket): void {
        winston.info('Web client is being registered');
        if (this.webClients.get(webClientSocket)) {
            throw new Error('Web client registration failed. It has already been registered');
        }

        const generatedID = ++this.idGenerator;
        this.webClients.set(webClientSocket, {
            id: generatedID,
            socket: webClientSocket,
            connectedPIDevices: [],
        });

        winston.info(`Web client has been registered with ID ${generatedID}`);
    }

    public addPIDevice(piDeviceSocket: WebSocket, name: string, pin: string): void {
        winston.info(`PI device "${name}" is being registered`);
        if (this.piDevices.get(piDeviceSocket)) {
            throw new Error(`PI device "${name}" registration failed. It has already been registered`);
        }

        const generatedID = ++this.idGenerator;
        this.piDevices.set(piDeviceSocket, {
            pin,
            name,
            users: [],
            id: generatedID,
            socket: piDeviceSocket,
        });

        winston.info(`PI device "${name}" has been registered with ID ${generatedID}`);
    }

    public getWebClient(webClientSocket: WebSocket): WebClient | undefined {
        return this.webClients.get(webClientSocket);
    }

    public getPIDevice(piDeviceSocket: WebSocket): PIDevice | undefined {
        return this.piDevices.get(piDeviceSocket);
    }

    public removeWebClient(webClientSocket: WebSocket): void {
        const webClient = this.webClients.get(webClientSocket);
        if (webClient) {
            winston.info(`Web client ${webClient.id} has been forgot`);
            this.webClients.delete(webClientSocket);
        }
    }

    public removePIDevice(piDeviceSocket: WebSocket): void {
        const piDevice = this.piDevices.get(piDeviceSocket);
        if (piDevice) {
            winston.info(`PI device ${piDevice.id} has been forgot`);
            this.piDevices.delete(piDeviceSocket);
        }
    }

    public getPIDeviceByID(deviceID: number): PIDevice | undefined {
        const [piDevice] = Array.from(this.piDevices.values())
            .filter((instance: PIDevice): boolean => (
                instance.id === deviceID
            ));

        return piDevice;
    }

    public piDevicesToArray(): PIDevice[] {
        return Array.from(this.piDevices.values());
    }

    public webClientsToArray(): WebClient[] {
        return Array.from(this.webClients.values());
    }
}

export default class MessageHandler {
    private storage: InMemoryStorage;
    public constructor() {
        this.storage = new InMemoryStorage();
    }

    private parseMessage(message: WebSocket.Data): InboundMessage<any> {
        if (typeof (message) !== 'string') {
            throw new Error('Bad message. Must be a JSON string');
        }

        let body: InboundMessage<any> | null = null;
        try {
            body = JSON.parse(message) as InboundMessage<any>;
        } catch {
            throw new Error('Bad message. Must be a JSON string');
        }

        if (typeof (body.type) !== 'string') {
            throw new Error('Bad message. Message type must be a string');
        }

        return body;
    }

    private updatePIUsersOnWebClient(
        webClient: WebClient,
        deviceID: number,
        users: User[],
    ): void {
        winston.verbose(`Web client ${webClient.id} is being notifed about users of PI device ${deviceID}`);
        const message: OutboundMessage<UpdatePIUsersPayload> = {
            type: OutboundMessageTypes.UPDATE_PI_USERS,
            payload: {
                deviceID,
                users,
            },
        };

        webClient.socket.send(JSON.stringify(message));
    }

    private updatePIUsersOnWebClients(
        deviceID: number,
        users: User[],
    ): void {
        const connectedWebClients = this.storage.webClientsToArray()
            .filter((webClient: WebClient): boolean => webClient.connectedPIDevices.includes(deviceID));
        for (const connectedWebClient of connectedWebClients) {
            this.updatePIUsersOnWebClient(
                connectedWebClient,
                deviceID,
                users,
            );
        }
    }

    private updatePIDevicesOnWebClient(
        webClient: WebClient,
    ): void {
        winston.verbose(`Web client ${webClient.id} is being notifed about current PI devices`);
        const message: OutboundMessage<UpdatePIDevicesPayload> = {
            type: OutboundMessageTypes.UPDATE_PI_DEVICES,
            payload: {
                devices: this.storage.piDevicesToArray().map((piDevice: PIDevice) => ({
                    id: piDevice.id,
                    name: piDevice.name,
                })),
            },
        };

        webClient.socket.send(JSON.stringify(message));
    }

    private updatePIDevicesOnWebClients(): void {
        for (const webClient of this.storage.webClientsToArray()) {
            this.updatePIDevicesOnWebClient(webClient);
        }
    }

    private answerError(socket: WebSocket, message: InboundMessage<any> | {type: 'UNKNOWN'}): void {
        const body: OutboundMessage<ErrorStatusPayload> = {
            type: OutboundMessageTypes.ERROR_STATUS,
            payload: {
                for: message.type,
            },
        };

        socket.send(JSON.stringify(body));
    }

    private answerOK(socket: WebSocket, message: InboundMessage<any>): void {
        const body: OutboundMessage<OKStatusPayload> = {
            type: OutboundMessageTypes.OK_STATUS,
            payload: {
                for: message.type,
            },
        };

        socket.send(JSON.stringify(body));
    }

    private handlePIDeviceConnection(
        piDeviceSocket: WebSocket,
        message: InboundMessage<ConnectPIPayload>)
    : void {
        const {
            pin,
            name,
        } = message?.payload;

        const valid = typeof(name) === 'string' && typeof(pin) === 'string';
        if (!valid) {
            throw new Error('Bad message. Data has invalid format for this message type');
        }

        this.storage.addPIDevice(piDeviceSocket, name, pin);
        this.answerOK(piDeviceSocket, message);

        piDeviceSocket.once('close', () => {
            this.storage.removePIDevice(piDeviceSocket);
            this.updatePIDevicesOnWebClients();
        });

        this.updatePIDevicesOnWebClients();
    }

    private handleWebClientConnection(webClientSocket: WebSocket, message: InboundMessage<undefined>): void {
        this.storage.addWebClient(webClientSocket);
        this.answerOK(webClientSocket, message);

        webClientSocket.once('close', () => {
            this.storage.removeWebClient(webClientSocket);
        });

        const webClient = this.storage.getWebClient(webClientSocket) as WebClient; // has been just added
        this.updatePIDevicesOnWebClient(webClient);
    }

    private handleUpdatePIUsers(
        piDeviceSocket: WebSocket,
        message: InboundMessage<UpdatePIUsersPayload>)
    : void {
        winston.info('PI device is updating its users');
        const piDevice = this.storage.getPIDevice(piDeviceSocket);
        if (!piDevice) {
            throw new Error('Bad request. PI device is unknown');
        }

        const {
            users,
        } = message.payload;

        const valid = Array.isArray(users) && users
            .every((user: User): boolean => userIsValid(user));
        if (!valid) {
            throw new Error('Bad message. Data has invalid format for this message type');
        }

        piDevice.users = users;
        winston.info(`Users have been updated for PI device ${piDevice.id}`);
        this.answerOK(piDeviceSocket, message);
        this.updatePIUsersOnWebClients(piDevice.id, piDevice.users);
    }

    private handleWebClientAuthorization(
        webClientSocket: WebSocket,
        message: InboundMessage<AuthorizeClientPayload>
    ): void {
        winston.info('Web client is connecting to a PI device');
        const webClient = this.storage.getWebClient(webClientSocket);
        if (!webClient) {
            throw new Error('Bad request. Web client is unknown');
        }

        const {
            deviceID,
            pin,
        } = message?.payload;
        const valid = typeof (deviceID) === 'number' && typeof (pin) === 'string';
        if (!valid) {
            throw new Error('Bad message. Data has invalid format for this message type');
        }

        const piDevice = this.storage.getPIDeviceByID(deviceID);
        if (!piDevice || piDevice.pin !== pin) {
            throw new Error(`The connection of web client ${webClient.id} to PI device ${deviceID} has been rejected`);
        } else {
            if (!webClient.connectedPIDevices.includes(deviceID)) {
                webClient.connectedPIDevices.push(deviceID);
            }
            winston.info(`The connection of web client ${webClient.id} to PI device ${deviceID} has been accepted`);
            this.answerOK(webClientSocket, message);
            this.updatePIUsersOnWebClient(webClient, piDevice.id, piDevice.users);
        }
    }

    private handleCreatePIUser(
        webClientSocket: WebSocket,
        message: InboundMessage<CreatePIUserPayload>
    ): void {
        winston.info('Web client is trying to create a PI user')
        const webClient = this.storage.getWebClient(webClientSocket);
        if (!webClient) {
            throw new Error('Bad request. Web client is unknown');
        }

        const {
            deviceID,
            user,
        } = message?.payload;
        const valid = userIsValid(user) && typeof(user.image) === 'string' && typeof(deviceID) === 'number';
        if (!valid) {
            throw new Error('Bad message. Data has invalid format for this message type');
        }

        const piDevice = this.storage.getPIDeviceByID(deviceID as number);
        if (!piDevice || !webClient.connectedPIDevices.includes(piDevice.id)) {
            throw new Error(`User has not been added to device ${deviceID}. Access rejected`);
        } else {
            const messageToPI: OutboundMessage<CreatePIUserPayload> = {
                type: OutboundMessageTypes.CREATE_PI_USER,
                payload: {
                    user,
                },
            };

            const waitForResult = (_message: WebSocket.Data) => {
                try {
                    const parsedMessage = this.parseMessage(_message);
                    if (parsedMessage.type === InboundMessageTypes.OK_STATUS) {
                        if ((parsedMessage.payload as OKStatusPayload).for === OutboundMessageTypes.CREATE_PI_USER) {
                            winston.info(`User has been created on PI device ${deviceID}`);
                            this.answerOK(webClientSocket, message);
                            return;
                        } else if ((parsedMessage.payload as ErrorStatusPayload).for === OutboundMessageTypes.CREATE_PI_USER) {
                            winston.error(`User has not been created on PI device ${deviceID}`);
                            this.answerError(webClientSocket, message);
                            return;
                        }
                    }
                } catch (_) {
                    // do nothing
                }

                piDevice.socket.once('message', waitForResult);
            };
            piDevice.socket.once('message', waitForResult);
            piDevice.socket.send(JSON.stringify(messageToPI));

            // after that PI device sends UPDATE_PI_USERS and the server makes UPDATE_PI_USERS for all clients
        }
    }

    private handleRemovePIUser(
        webClientSocket: WebSocket,
        message: InboundMessage<RemovePIUserPayload>
    ): void {
        winston.info('Web client is trying to remove a PI user')
        const webClient = this.storage.getWebClient(webClientSocket);
        if (!webClient) {
            throw new Error('Bad request. Web client is unknown');
        }

        const {
            deviceID,
            userID,
        } = message?.payload;
        const valid = typeof(deviceID) === 'number' && typeof(userID) === 'number';
        if (!valid) {
            throw new Error('Bad message. Data has invalid format for this message type');
        }

        const piDevice = this.storage.getPIDeviceByID(deviceID as number);
        if (!piDevice || !webClient.connectedPIDevices.includes(piDevice.id)) {
            throw new Error(`User ${userID} has not been removed from device ${deviceID}. Access rejected`);
        } else {
            if (!piDevice.users.map((user: User): number => user.id as number).includes(userID)) {
                throw new Error(`User has not been removed from device ${deviceID}. No such the user ID`);
            }
            const messageToPI: OutboundMessage<RemovePIUserPayload> = {
                type: OutboundMessageTypes.REMOVE_PI_USER,
                payload: {
                    userID,
                },
            };

            const waitForResult = (_message: WebSocket.Data) => {
                try {
                    const parsedMessage = this.parseMessage(_message);
                    if (parsedMessage.type === InboundMessageTypes.OK_STATUS) {
                        if ((parsedMessage.payload as OKStatusPayload).for === OutboundMessageTypes.REMOVE_PI_USER) {
                            winston.info(`User has been created on PI device ${deviceID}`);
                            this.answerOK(webClientSocket, message);
                            return;
                        } else if ((parsedMessage.payload as ErrorStatusPayload).for === OutboundMessageTypes.REMOVE_PI_USER) {
                            winston.error(`User has not been created on PI device ${deviceID}`);
                            this.answerError(webClientSocket, message);
                            return;
                        }
                    }
                } catch (_) {
                    // do nothing
                }

                piDevice.socket.once('message', waitForResult);
            };
            piDevice.socket.once('message', waitForResult);
            piDevice.socket.send(JSON.stringify(messageToPI));

            // after that PI device sends UPDATE_PI_USERS and the server makes UPDATE_PI_USERS for all clients
        }
    }

    private handleMessage(
        socket: WebSocket,
        message: InboundMessage<any>
    ): void {
        switch (message.type) {
            case InboundMessageTypes.CONNECT_PI: {
                this.handlePIDeviceConnection(socket, message);
                break;
            }
            case InboundMessageTypes.CONNECT_WEB_CLIENT: {
                this.handleWebClientConnection(socket, message);
                break;
            }
            case InboundMessageTypes.UPDATE_PI_USERS: {
                this.handleUpdatePIUsers(socket, message)
                break;
            }
            case InboundMessageTypes.AUTHORIZE_CLIENT: {
                this.handleWebClientAuthorization(socket, message);
                break;
            }
            case InboundMessageTypes.CREATE_PI_USER: {
                this.handleCreatePIUser(socket, message);
                break;
            }
            case InboundMessageTypes.REMOVE_PI_USER: {
                this.handleRemovePIUser(socket, message);
                break;
            }
            default: {
                throw new Error(`Unknown inbound message type: ${message.type}`);
            }
        }
    }

    public handle = (
        socket: WebSocket,
        message: WebSocket.Data,
    ): void => {
        let parsedMessage = null;
        try {
            winston.info('Server has received a message');
            parsedMessage = this.parseMessage(message);
            this.handleMessage(socket, parsedMessage);
        } catch (error) {
            this.answerError(socket, parsedMessage ? parsedMessage : {
                type: 'UNKNOWN',
            });
            winston.error(error.toString());
        }
    }
}