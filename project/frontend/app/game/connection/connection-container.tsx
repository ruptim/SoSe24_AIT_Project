import {Badge} from "@nextui-org/badge";


type ConnectionContainerParams = {
  isConnected: boolean;
}

export function ConnectionContainer({isConnected}: ConnectionContainerParams){
  return (
    <div className="flex flex-row">
      <small>Connection</small>
      <Badge content={" "} size="sm" color={isConnected ? "success" : "danger"}>
        <div className="w-2">&nbsp;</div>
      </Badge>
    </div>
  )
}