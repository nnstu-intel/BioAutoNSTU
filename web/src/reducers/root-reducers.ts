import { combineReducers } from 'redux';

import devicesReducer from './devices-reducer';
import usersReducer from './users-reducer';

export default combineReducers({
    devices: devicesReducer,
    users: usersReducer,
});
