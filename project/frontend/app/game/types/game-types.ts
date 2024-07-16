export type BuzzerType = {
  buzzerId: number;
  buzzerName: string;
  isPressed: boolean;
  isLocked: boolean;
  delay: number | null;
};

export type QuestionOption = {
  optionText: string;
  isCorrect: boolean;
}

export type QuestionType = {
  question: string;
  options: QuestionOption[] | null;
  answer: string;
};
