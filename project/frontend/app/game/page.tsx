import { title } from "@/components/primitives";
import {QuestionContainer} from "@/app/game/question/question-container";
import {Divider} from "@nextui-org/divider";
import {BuzzerContainer} from "@/app/game/buzzer/buzzer-container";
import {ManagementContainer} from "@/app/game/management/management-container";
import {ActivityContainer} from "@/app/game/activity/activity-container";
import {BuzzerType} from "@/app/game/types/game-types";

export default function GamePage() {

    let buzzerList: BuzzerType[] = [
        {
            buzzerId: 0,
            buzzerName: 'First Buzzer',
            isPressed: true,
            isLocked: false,
            delay: 2.56
        },
        {
            buzzerId: 1,
            buzzerName: 'Second Buzzer',
            isPressed: false,
            isLocked: false,
            delay: null
        }
    ]

  return (
    <div>
        <QuestionContainer></QuestionContainer>
        <Divider className={"mt-5 mb-5"}></Divider>
        <ActivityContainer buzzerList={buzzerList}></ActivityContainer>
    </div>
  );
}
