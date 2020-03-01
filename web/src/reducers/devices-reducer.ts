export interface Device {
    id: number;
    name: string;
}

export interface DevicesReducer {
    fetching: boolean;
    active: number | null;
    list: Device[];
}

const defaultState: DevicesReducer = {
    fetching: false,
    active: 5,
    list: [{
        id: 5,
        name: 'Raspberry PI 4 Model B',
    }, {
        id: 1,
        name: 'Raspberry PI 3 Model B',
    }],
}

export default function (state = defaultState): DevicesReducer {
    return state;
}
