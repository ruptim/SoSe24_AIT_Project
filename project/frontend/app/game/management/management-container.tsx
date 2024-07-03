import {ConnectBuzzerButton} from "@/app/game/management/connect-buzzer-button";
import {EditBuzzersButton} from "@/app/game/management/edit-buzzers-button";
import {useState} from "react";
import {BuzzerType} from "@/app/game/types/game-types";

type ManagementContainerParams = {
    buzzers: BuzzerType[],
}

export function ManagementContainer({buzzers}: ManagementContainerParams){

    const [isPairing, setPairing] = useState(false);
    const [buzzerArr, setBuzzerArr] = useState<BuzzerType[]>(buzzers);
    const [newBuzzerArr, setNewBuzzerArr] = useState<BuzzerType[]>([]);

    function connectModalOpened(){
        setPairing(true);
        getNewBuzzers();
    }

    function connectModalClosed(){
        setPairing(false);
    }

    function getNewBuzzers(){
        setNewBuzzerArr([{
            buzzerId: 2,
            buzzerName: 'New Buzzer',
            isPressed: false,
            isLocked: false,
            delay: null
        }])
    }

    return (
        <div className={"flex flex-row gap-5 justify-start"}>
            <ConnectBuzzerButton onOpenClicked={connectModalOpened} isPairing={isPairing} onModalClosed={connectModalClosed} buzzersShown={newBuzzerArr}></ConnectBuzzerButton>
            <EditBuzzersButton></EditBuzzersButton>
        </div>
    )
}