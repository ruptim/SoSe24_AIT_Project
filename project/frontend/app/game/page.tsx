import { title } from "@/components/primitives";
import {QuestionContainer} from "@/app/game/question/question-container";
import {Divider} from "@nextui-org/divider";
import {BuzzerContainer} from "@/app/game/buzzer/buzzer-container";
import {ManagementContainer} from "@/app/game/management/management-container";

export default function DocsPage() {
  return (
    <div>
        <QuestionContainer></QuestionContainer>
        <Divider className={"mt-5 mb-5"}></Divider>
        <BuzzerContainer></BuzzerContainer>
        <Divider className={"mt-5 mb-5"}></Divider>
        <ManagementContainer></ManagementContainer>
    </div>
  );
}
