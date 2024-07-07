'use client';

import {Button} from "@nextui-org/button";

type BuzzerLockButtonParams = {
    onLockClick: () => void,
    isActive: boolean
}

export function BuzzerLockButton({onLockClick, isActive}: BuzzerLockButtonParams){

    return (
        <Button color="warning" onClick={onLockClick}>{isActive ? 'Unlock All' : 'Lock All'}</Button>
    )
}