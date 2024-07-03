'use client';

import {Buzzer} from "@/app/game/buzzer/buzzer";
import {BuzzerResetButton} from "@/app/game/buzzer/buzzer-reset-button";
import {BuzzerLockButton} from "@/app/game/buzzer/buzzer-lock-button";
import {BuzzerType} from "@/app/game/types/game-types";

type BuzzerContainerParams = {
    buzzers: BuzzerType[],
    isAllLocked: boolean,
    onResetClick: () => void,
    onLockClick: () => void
}

export function BuzzerContainer({buzzers, onResetClick, onLockClick, isAllLocked}: BuzzerContainerParams){

    return (
        <div>
            <div className={"flex flex-row justify-center gap-5"}>
                {buzzers.map(buzzer => (
                    <div className={"box-border h-32 w-32"} key={buzzer.buzzerId}>
                        <Buzzer buzzerId={buzzer.buzzerId} buzzerName={buzzer.buzzerName} isLocked={buzzer.isLocked} isPressed={buzzer.isPressed} delay={buzzer.delay}></Buzzer>
                    </div>
                ))}
            </div>
            <div className={"flex flex-row justify-center gap-5"}>
                <BuzzerResetButton onResetClick={onResetClick}></BuzzerResetButton>
                <BuzzerLockButton onLockClick={onLockClick} isActive={isAllLocked}></BuzzerLockButton>
            </div>
        </div>
    )
}