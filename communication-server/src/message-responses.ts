import WebSocket from 'ws';

import {
    OutboundMessage,
    OutboundMessageTypes,
    SendPIDevicesPayload,
    SendPIUsersPayload,
    RegisteredPIInstance,
} from './types';

export function notifyBadProtocol(webClientSocket: WebSocket) {
    const body = {
        type: OutboundMessageTypes.BAD_PROTOCOL,
    } as OutboundMessage<undefined>;

    webClientSocket.send(JSON.stringify(body));
}

export function notifyBadAuthorization(webClientSocket: WebSocket) {
    const body = {
        type: OutboundMessageTypes.BAD_AUTHORIZATION,
    } as OutboundMessage<undefined>;

    webClientSocket.send(JSON.stringify(body));
}


export function sendPIDevices(
    webClientSocket: WebSocket, 
    registeredPIInstances: RegisteredPIInstance[],
) {
    const body = {
        type: OutboundMessageTypes.SEND_PI_DEVICES,
        payload: {
            devices: registeredPIInstances.map((instance: RegisteredPIInstance) => ({
                id: instance.id,
                name: instance.name,
            })),
        },
    } as OutboundMessage<SendPIDevicesPayload>;

    webClientSocket.send(JSON.stringify(body));
}

export function sendPIUsers(
    webClientSocket: WebSocket, 
    registeredPIInstance: RegisteredPIInstance,
) {
    const body = {
        type: OutboundMessageTypes.SEND_PI_USERS,
        payload: {
            id: registeredPIInstance.id,
            users: registeredPIInstance.users,
        },
    } as OutboundMessage<SendPIUsersPayload>;

    webClientSocket.send(JSON.stringify(body));
}
