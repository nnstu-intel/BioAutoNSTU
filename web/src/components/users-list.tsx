import React from 'react';
import { connect } from 'react-redux';
import { Spin, Card, Empty, Tooltip } from 'antd';
import { UserAddOutlined } from '@ant-design/icons';

import { State } from 'store';
import { User } from 'reducers/users-reducer';

import UserItem from './user-item';

interface StateToProps {
    fetching: boolean;
    users: User[];
}

function mapStateToProps(state: State): StateToProps {
    const {
        users: {
            fetching,
            lists,
        },
        devices: {
            active: activeDevice,
        },
    } = state;

    return {
        fetching,
        users: (activeDevice ? lists[activeDevice] : []) || [],
    };
}

function UsersList(props: StateToProps): JSX.Element {
    const { fetching, users } = props;
    const title = 'Users on the device';

    if (fetching) {
        return (
            <Card title={title} bordered>
                <div className='users-fetching-spin-wrapper'>
                    <Spin size='large' />
                </div>
            </Card>
        );
    }

    if (!users.length) {
        return (
            <Card title={title} bordered>
                <Empty />
            </Card>
        );
    }

    return (
        <Card
            title={title}
            bordered
            extra={
                <Tooltip title='Add a new user'>
                    <UserAddOutlined className='add-new-user-icon' />
                </Tooltip>
            }
        >
            {users.map((user: User): JSX.Element => (
                <UserItem key={user.id} user={user} />
            ))}
        </Card>
    );
}

export default connect(
    mapStateToProps,
)(UsersList);
