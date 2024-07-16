"use client";
import { Accordion, AccordionItem } from "@nextui-org/react";

import { title } from "@/components/primitives";
import {Chip} from "@nextui-org/chip";
import {QuestionOption} from "@/app/game/types/game-types";

type questionParams = {
  question: string;
  answer: string;
  options: QuestionOption[] | null;
  isExpanded: boolean;
  onExpansionChange: () => void
};

export function Question({ question, answer, options, isExpanded, onExpansionChange }: questionParams) {
  let defaultExpandedKey = isExpanded ? '1' : '';

  function indexToLetter(index: number){
    switch (index) {
      case 1:
        return 'B';
      case 2:
        return 'C';
      case 3:
        return 'D';
      default:
        return 'A';
    }
  }

  return (
    <Accordion className="w-full min-w-full text-center " selectedKeys={[defaultExpandedKey]} onSelectionChange={onExpansionChange}>
      <AccordionItem key={'1'} title={<>
        <p className={title()}>{question}</p>
        <div className="flex flex-row justify-center gap-5 mt-10">
          {options?.map((option, index) => (
            <div key={index}>
              <Chip key={index} color="secondary" variant={(isExpanded && option.isCorrect) ? "solid" : "bordered"} size="lg">{indexToLetter(index)}: {option.optionText}</Chip>
            </div>
          ))}
        </div>
      </>
        } >
        <div className="max-w-prose text-left">{answer}</div>
      </AccordionItem>
    </Accordion>
  );
}
