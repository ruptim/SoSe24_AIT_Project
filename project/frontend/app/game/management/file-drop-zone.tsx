import { Dropzone, FileMosaic } from "@files-ui/react";
import {SetStateAction, useEffect, useState} from "react";
import {ExtFile} from '@files-ui/core'

export function FileDropZone() {
  const [files, setFiles] = useState<ExtFile[]>([]);
  const [parsedFile, setParsedFile] = useState(null);
  const fileReader = new FileReader();

  useEffect(() => {
  }, [files]);

  const updateFiles = (incomingFiles: SetStateAction<ExtFile[]>) => {
    //do something with the files
    console.log("incoming files", incomingFiles);
    setFiles(incomingFiles);
    //even your own upload implementation
  };

  const removeFile = (id: any) => {
    setFiles(files.filter((x) => x.id !== id));
  };

  return (
    <Dropzone
      onChange={updateFiles}
      value={files}
      accept=".json"
    >
      {files.map((file) => (
        <FileMosaic key={file.id} {...file} onDelete={removeFile} info />
      ))}
    </Dropzone>
  );
}