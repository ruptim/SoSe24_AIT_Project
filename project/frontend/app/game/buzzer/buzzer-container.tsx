'use client';

import {Buzzer} from "@/app/game/buzzer/buzzer";
import {BuzzerResetButton} from "@/app/game/buzzer/buzzer-reset-button";
import {BuzzerLockButton} from "@/app/game/buzzer/buzzer-lock-button";
import {BuzzerType} from "@/app/game/types/game-types";
import {compareBuzzerDelay} from "@/app/game/types/compare-buzzer-delay";

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
                {buzzers.sort((a, b) => compareBuzzerDelay(a, b)).map((buzzer, index) => (
                    <div className={"box-border h-32 w-40"} key={buzzer.buzzerId}>
                        <Buzzer buzzerRank={index + 1} buzzerName={buzzer.buzzerName} isLocked={buzzer.isLocked} isPressed={buzzer.isPressed} delay={buzzer.delay}></Buzzer>
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