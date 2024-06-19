import {QuestionButton} from "@/app/game/question/question-button";
import {QuestionCounter} from "@/app/game/question/question-counter";

export function QuestionContainer(){
    return (
        <div>
            <div>Question 1: Lorem Ipsum</div>
            <div>
                <QuestionButton isSkip={false}></QuestionButton>
                <QuestionButton isSkip={true}></QuestionButton>
            </div>
            <div>Question <QuestionCounter currentCount={1} maxCount={10}></QuestionCounter></div>
        </div>
    )
}