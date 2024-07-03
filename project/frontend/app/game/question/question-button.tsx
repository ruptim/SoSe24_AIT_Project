import {Button} from "@nextui-org/button";

type QuestionButtonParams = {
    isSkip: boolean,
    onButtonClick: () => void,
    isEnabled: boolean
}

export function QuestionButton({isSkip, onButtonClick, isEnabled}: QuestionButtonParams){

    return (
        <Button color="primary" className={"w-full"} onClick={onButtonClick} isDisabled={!isEnabled}>
            {isSkip ? (
                <span>Next &gt;&gt;</span>
            ) : (
                <span>&lt;&lt; Previous</span>
            )}
        </Button>
    )
}