import {Button} from "@nextui-org/button";

type QuestionButtonParams = {
    isSkip: boolean
}

export function QuestionButton({isSkip}: QuestionButtonParams){

    return (
        <Button color="primary">
            {isSkip ? (
                <span>Next</span>
            ) : (
                <span>Previous</span>
            )}
        </Button>
    )
}