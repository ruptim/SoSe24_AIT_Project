import {ConnectBuzzerButton} from "@/app/game/management/connect-buzzer-button";
import {EditBuzzersButton} from "@/app/game/management/edit-buzzers-button";

export function ManagementContainer(){
    return (
        <div className={"flex flex-row gap-5 justify-start"}>
            <ConnectBuzzerButton></ConnectBuzzerButton>
            <EditBuzzersButton></EditBuzzersButton>
        </div>
    )
}