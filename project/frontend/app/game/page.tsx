import { title } from "@/components/primitives";
import {QuestionContainer} from "@/app/game/question/question-container";
import {Divider} from "@nextui-org/divider";
import {BuzzerContainer} from "@/app/game/buzzers/buzzer-container";

export default function DocsPage() {
  return (
    <div>
      <h1 className={title()}>Game</h1>
        <QuestionContainer></QuestionContainer>
        <Divider></Divider>
        <BuzzerContainer></BuzzerContainer>
    </div>
  );
}
