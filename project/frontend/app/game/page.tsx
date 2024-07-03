import { title } from "@/components/primitives";
import {QuestionContainer} from "@/app/game/question/question-container";
import {Divider} from "@nextui-org/divider";
import {BuzzerContainer} from "@/app/game/buzzer/buzzer-container";
import {ManagementContainer} from "@/app/game/management/management-container";
import {Activity} from "@/app/game/activity/activity";

export default function GamePage() {
  return (
    <div>
        <QuestionContainer></QuestionContainer>
        <Divider className={"mt-5 mb-5"}></Divider>
        <Activity></Activity>
    </div>
  );
}
