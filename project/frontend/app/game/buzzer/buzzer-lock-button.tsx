'use client';

import {Button} from "@nextui-org/button";

type BuzzerLockButtonParams = {
    onLockClick: () => void;
}

export function BuzzerLockButton({onLockClick}: BuzzerLockButtonParams){

    return (
        <Button color="warning" onClick={onLockClick}>Lock All</Button>
    )
}