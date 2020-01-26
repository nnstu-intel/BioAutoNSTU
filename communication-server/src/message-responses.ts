import WebSocket from 'ws';

import {
    OutboundMessage,
    OutboundMessageTypes,
    SendPIDevicesPayload,
    SendPIUsersPayload,
    RegisteredPIInstance,

    User,
    CreatePIUserPayload,
    RemovePIUserPayload,
} from './types';

export function notifyBadProtocol(webClientSocket: WebSocket) {
    const body = {
        type: OutboundMessageTypes.BAD_PROTOCOL,
    } as OutboundMessage<undefined>;

    webClientSocket.send(JSON.stringify(body));
}

export function notifyBadAuthorization(webClientSocket: WebSocket): void {
    const body = {
        type: OutboundMessageTypes.BAD_AUTHORIZATION,
    } as OutboundMessage<undefined>;

    webClientSocket.send(JSON.stringify(body));
}

export function sendPIDevices(
    webClientSocket: WebSocket, 
    registeredPIInstances: RegisteredPIInstance[],
): void {
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
): void {
    const body = {
        type: OutboundMessageTypes.SEND_PI_USERS,
        payload: {
            id: registeredPIInstance.id,
            users: registeredPIInstance.users,
        },
    } as OutboundMessage<SendPIUsersPayload>;

    webClientSocket.send(JSON.stringify(body));
}

export function sendCreatePIUser(
    PIDeviceSocket: WebSocket,
    user: User,
): void {
    const body = {
        type: OutboundMessageTypes.CREATE_PI_USER,
        payload: {
            user,
        },
    } as OutboundMessage<CreatePIUserPayload>;

    PIDeviceSocket.send(JSON.stringify(body));
}

export function sendRemovePIUser(
    PIDeviceSocket: WebSocket,
    userID: number,
): void {
    const body = {
        type: OutboundMessageTypes.SEND_PI_USERS,
        payload: {
            userID,
        },
    } as OutboundMessage<RemovePIUserPayload>;

    PIDeviceSocket.send(JSON.stringify(body));
}