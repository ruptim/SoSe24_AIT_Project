import {Buzzer} from "@/app/game/buzzers/buzzer";
import {BuzzerResetButton} from "@/app/game/buzzers/buzzer-reset-button";

export function BuzzerContainer(){
    return (
        <div>
            <div className={"flex flex-row justify-center gap-5"}>
                <div className={"box-border h-32 w-32"}>
                    <Buzzer buzzerId={0} buzzerName={'Buzzer Name'} isPressed={false}></Buzzer>
                </div>
                <div className={"box-border h-32 w-32"}>
                    <Buzzer buzzerId={1} buzzerName={'A Player'} isPressed={true}></Buzzer>
                </div>
                <div className={"box-border h-32 w-32"}>
                    <Buzzer buzzerId={2} buzzerName={'Second Player'}
                            isPressed={false}></Buzzer>
                </div>
            </div>
            <BuzzerResetButton></BuzzerResetButton>
        </div>
    )
}