import {QuestionButton} from "@/app/game/question/question-button";
import {QuestionCounter} from "@/app/game/question/question-counter";
import {title} from "@/components/primitives";

export function QuestionContainer(){
    let question = "Lorem Ipsum dolor sit amet?"
    return (
        <div>
            <h1 className={title()}>{question}</h1>
            <div className={"flex mt-10 mb-2 justify-center gap-2"}>
                <div className={"box-border w-1/5"}><QuestionButton isSkip={false}></QuestionButton></div>
                <div className={"box-border w-1/5"}><QuestionButton isSkip={true}></QuestionButton></div>
            </div>
            <div className={"flex justify-end"}>Question&nbsp;<QuestionCounter currentCount={1} maxCount={10}></QuestionCounter></div>
        </div>
    )
}