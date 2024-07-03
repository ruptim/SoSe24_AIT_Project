import {Buzzer} from "@/app/game/buzzer/buzzer";
import {BuzzerResetButton} from "@/app/game/buzzer/buzzer-reset-button";
import {BuzzerLockButton} from "@/app/game/buzzer/buzzer-lock-button";
import {BuzzerType} from "@/app/game/types/game-types";

type BuzzerContainerParams = {
    buzzers: BuzzerType[]
}

export function BuzzerContainer({buzzers}: BuzzerContainerParams){
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
                <BuzzerResetButton></BuzzerResetButton>
                <BuzzerLockButton></BuzzerLockButton>
            </div>
        </div>
    )
}