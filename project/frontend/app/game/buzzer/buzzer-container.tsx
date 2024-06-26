import {Buzzer} from "@/app/game/buzzer/buzzer";
import {BuzzerResetButton} from "@/app/game/buzzer/buzzer-reset-button";
import {BuzzerLockButton} from "@/app/game/buzzer/buzzer-lock-button";

export function BuzzerContainer(){
    return (
        <div>
            <div className={"flex flex-row justify-center gap-5"}>
                <div className={"box-border h-32 w-32"}>
                    <Buzzer buzzerId={1} buzzerName={'Buzzer Name'} timestamp={new Date('2020-05-14T04:00:01.001Z')} isPressed={false} isLocked={false}></Buzzer>
                </div>
                <div className={"box-border h-32 w-32"}>
                    <Buzzer buzzerId={2} buzzerName={'A Player'} timestamp={new Date('2020-05-14T04:00:10.001Z')} isPressed={true} isLocked={false}></Buzzer>
                </div>
                <div className={"box-border h-32 w-32"}>
                    <Buzzer buzzerId={3} buzzerName={'Second Player'} timestamp={new Date('2020-05-14T04:00:03.101Z')}
                            isPressed={false} isLocked={true}></Buzzer>
                </div>
            </div>
            <div className={"flex flex-row justify-center gap-5"}>
                <BuzzerResetButton></BuzzerResetButton>
                <BuzzerLockButton></BuzzerLockButton>
            </div>
        </div>
    )
}