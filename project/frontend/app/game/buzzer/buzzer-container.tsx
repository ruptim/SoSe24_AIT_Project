'use client';

import {Buzzer} from "@/app/game/buzzer/buzzer";
import {BuzzerResetButton} from "@/app/game/buzzer/buzzer-reset-button";
import {BuzzerLockButton} from "@/app/game/buzzer/buzzer-lock-button";
import {BuzzerType} from "@/app/game/types/game-types";
import {MouseEventHandler} from "react";

type BuzzerContainerParams = {
    buzzers: BuzzerType[],
    onResetClick: () => void,
    onLockClick: () => void
}

export function BuzzerContainer({buzzers, onResetClick, onLockClick}: BuzzerContainerParams){


    return (
        <div>
            <div className={"flex flex-row justify-center gap-5"}>
                {buzzers.map(buzzer => (
                    <div className={"box-border h-32 w-32"} key={buzzer.buzzerId}>
                        <Buzzer buzzer={buzzer}></Buzzer>
                    </div>
                ))}
            </div>
            <div className={"flex flex-row justify-center gap-5"}>
                <BuzzerResetButton onResetClick={onResetClick}></BuzzerResetButton>
                <BuzzerLockButton onLockClick={onLockClick}></BuzzerLockButton>
            </div>
        </div>
    )
}