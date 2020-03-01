import { createStore } from 'redux';

import rootReducer from './reducers/root-reducers';

const store = createStore(rootReducer);
const state = store.getState();

export type State = typeof state;

export default function getStore() {
    return store;
}
