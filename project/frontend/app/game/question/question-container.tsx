"use client";

import { useEffect, useState } from "react";

import { QuestionButton } from "@/app/game/question/question-button";
import { QuestionCounter } from "@/app/game/question/question-counter";
import { Question } from "@/app/game/question/question";
import { QuestionType } from "@/app/game/types/game-types";

type QuestionContainerParams = {
  questions: QuestionType[];
};

export function QuestionContainer({ questions }: QuestionContainerParams) {
  let maxQuestions = questions.length;
  let [currentQuestion, setCurrentQuestion] = useState<QuestionType>(
    questions[0],
  );
  let [currentCount, setCurrentCount] = useState(0);

  function increaseCount() {
    if (currentCount + 1 < maxQuestions) {
      setCurrentCount(currentCount + 1);
    }
  }

  function decreaseCount() {
    if (currentCount) {
      setCurrentCount(currentCount - 1);
    }
  }

  useEffect(() => {
    setCurrentQuestion(questions[currentCount]);
  }, [currentCount]);

  return (
    <div className="flex flex-col w-full min-w-full">
      <Question
        answer={currentQuestion.answer}
        question={currentQuestion.question}
      />
      <div className={"flex mt-10 mb-2 justify-center gap-5 w-full"}>
        <div className={"box-border w-1/4"}>
          <QuestionButton
            isEnabled={currentCount > 0}
            isSkip={false}
            onButtonClick={decreaseCount}
          />
        </div>
        <div className={"box-border w-1/4"}>
          <QuestionButton
            isEnabled={currentCount + 1 < maxQuestions}
            isSkip={true}
            onButtonClick={increaseCount}
          />
        </div>
      </div>
      <div className={"flex justify-end"}>
        Question&nbsp;
        <QuestionCounter
          currentCount={currentCount + 1}
          maxCount={maxQuestions}
        />
      </div>
    </div>
  );
}
