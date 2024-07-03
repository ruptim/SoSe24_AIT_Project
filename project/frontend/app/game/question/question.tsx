'use client';
import {Accordion, AccordionItem} from "@nextui-org/react";
import {title} from "@/components/primitives";

type questionParams = {
    question: string,
    answer: string
}

export function Question({question, answer}: questionParams){
    return (
        <Accordion className="w-full min-w-full text-center">
            <AccordionItem key={1} title={<p className={title()}>{question}</p>}>
                <div className="max-w-prose text-left">
                    {answer}
                </div>
            </AccordionItem>
        </Accordion>
    )
}