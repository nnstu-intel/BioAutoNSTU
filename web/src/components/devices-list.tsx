import React from 'react';
import { connect } from 'react-redux';
import { Spin, Card, Empty } from 'antd';

import { Device } from 'reducers/devices-reducer';
import { State } from 'store';

import DeviceItem from './device-item';

interface StateToProps {
    fetching: boolean;
    devices: Device[];
    active: number | null;
}

function mapStateToProps(state: State): StateToProps {
    const {
        devices: {
            fetching,
            list: devices,
            active,
        }
    } = state;

    return {
        fetching,
        devices,
        active,
    };
}

function DevicesList(props: StateToProps): JSX.Element {
    const { fetching, devices, active } = props;
    const title = 'Raspberry Devices';
    if (fetching) {
        return (
            <Card title={title} bordered>
                <div className='devices-fetching-spin-wrapper'>
                    <Spin size='large' />
                </div>
            </Card>

        );
    }

    if (!devices.length) {
        return (
            <Card title={title} bordered>
                <Empty />
            </Card>
        );
    }

    return (
        <Card title={title} bordered>
            {devices.map((device: Device): JSX.Element => (
                <DeviceItem key={device.id} active={active === device.id} device={device} />
            ))}
        </Card>
    );
}

export default connect(
    mapStateToProps,
)(DevicesList);
