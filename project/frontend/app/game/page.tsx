import { title } from "@/components/primitives";
import {QuestionContainer} from "@/app/game/question/question-container";
import {Divider} from "@nextui-org/divider";
import {BuzzerContainer} from "@/app/game/buzzers/buzzer-container";

export default function DocsPage() {
  return (
    <div>
        <QuestionContainer></QuestionContainer>
        <Divider className={"mt-5 mb-5"}></Divider>
        <BuzzerContainer></BuzzerContainer>
    </div>
  );
}
