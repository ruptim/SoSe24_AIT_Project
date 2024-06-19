import {Buzzer} from "@/app/game/buzzers/buzzer";
import {BuzzerResetButton} from "@/app/game/buzzers/buzzer-reset-button";

export function BuzzerContainer(){
    return (
        <div>
            <div className={"flex flex-row justify-between"}>
                <Buzzer buzzerId={0} buzzerName={'Buzzer Name'} isPressed={false}></Buzzer>
                <Buzzer buzzerId={1} buzzerName={'A Player'} isPressed={true}></Buzzer>
                <Buzzer buzzerId={2} buzzerName={'Second Player'} isPressed={false}></Buzzer>
            </div>
            <BuzzerResetButton></BuzzerResetButton>
        </div>
    )
}