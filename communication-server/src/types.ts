import WebSocket from 'ws';

export enum InboundMessageTypes {
    CONNECT_PI = 'CONNECT_PI',
    CONNECT_WEB_CLIENT = 'CONNECT_WEB_CLIENT',
    AUTHORIZE_CLIENT = 'AUTHORIZE_CLIENT',
    CREATE_PI_USER = 'CREATE_PI_USER',
    REMOVE_PI_USER = 'REMOVE_PI_USER',
    PI_ERROR = 'PI_ERROR',
}

export enum OutboundMessageTypes {
    BAD_PROTOCOL = 'BAD_PROTOCOL',
    BAD_AUTHORIZATION = 'BAD_AUTHORIZATION',
    CREATE_PI_USER = 'CREATE_PI_USER',
    REMOVE_PI_USER = 'REMOVE_PI_USER',
    SEND_PI_DEVICES = 'SEND_PI_DEVICES',
    SEND_PI_USERS = 'SEND_PI_USERS',
}

export interface User {
    id?: number;
    passport: string;
    firstname: string;
    secondname: string;
    patronymic?: string;
    image?: string; // base64 encoded jpeg
}

export interface ConnectPIPayload {
    name: string;
    pin: number;
    users: User[];
}

export interface AuthorizeClientPayload {
    PIDeviceID: number;
    pin: number;
}

export interface CreatePIUserPayload {
    PIDeviceID: number;
    user: User;
}

export interface RemovePIUserPayload {
    PIDeviceID: number;
    userID: number;
}

export interface PIErrorPayload {
    error: string;
}

export interface InboundMessage<PayloadType> {
    type: InboundMessageTypes,
    payload: PayloadType;
}

export interface SendPIDevicesPayload {
    devices: {
        id: number;
        name: string;
    }[];
}

export interface SendPIUsersPayload {
    id: number;
    users: User[];
}

export interface OutboundMessage<PayloadType> {
    type: OutboundMessageTypes;
    payload: PayloadType;
}


export interface RegisteredPIInstance {
    id: number;
    pin: number;
    name: string;
    users: User[];
    socket: WebSocket;
}

export interface RegisteredWebClient {
    isAuthorizedFor: number[];
    id: number;
    socket: WebSocket;
}
