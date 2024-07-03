'use client';

import {Button} from "@nextui-org/button";

type BuzzerResetButtonParams = {
    onResetClick: () => void;
}

export function BuzzerResetButton({onResetClick}: BuzzerResetButtonParams){
    return (
        <Button color="danger" onClick={onResetClick}>Reset All</Button>
    )
}