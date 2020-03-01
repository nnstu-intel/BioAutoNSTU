export interface UpdatePIUsersPayload { // InboundMessageTypes.UPDATE_PI_USERS (without deviceID), // OutboundMessageTypes.UPDATE_PI_USERS
    deviceID?: number;
    users: User[];
}

export interface User {
    id: number;
    passport: string;
    firstname: string;
    secondname: string;
    patronymic?: string;
}

export interface UsersReducer {
    fetching: boolean;
    active: number | null;
    lists: Record<number, User[]>;
}

const defaultState: UsersReducer = {
    fetching: false,
    active: 1,
    lists: {
        5: [{
            id: 0,
            passport: '670906',
            firstname: 'Boris',
            secondname: 'Sekachev',
            patronymic: 'Sergeevich',
        }, {
            id: 1,
            passport: '670907',
            firstname: 'Boris',
            secondname: 'Sekachev',
        }],
        1: [{
            id: 2,
            passport: '670908',
            firstname: 'Boris',
            secondname: 'Sekachev',
            patronymic: 'Sergeevich',
        }],
    },
}

export default function (state = defaultState): UsersReducer {
    return state;
}
