import {Button} from "@nextui-org/button";

type QuestionButtonParams = {
    isSkip: boolean
}

export function QuestionButton({isSkip}: QuestionButtonParams){

    return (
        <Button color="primary" className={"w-full"}>
            {isSkip ? (
                <span>Next &gt;&gt;</span>
            ) : (
                <span>&lt;&lt; Previous</span>
            )}
        </Button>
    )
}