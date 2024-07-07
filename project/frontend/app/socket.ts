import io from 'socket.io-client'
import {backendConfig} from "@/config/backend-config";

const URL = backendConfig.hostUrl

export const socket = io(URL);