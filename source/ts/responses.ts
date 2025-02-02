export interface ComponentCountResponse {
  count: number;
}

export interface ScreenshotResponse {
  image: string;
}

export interface ComponentVisibilityResponse {
  showing: boolean;
  exists: boolean;
}

export interface ComponentEnablementResponse {
  enabled: boolean;
  exists: boolean;
}

export interface ComponentTextResponse {
  text: string;
}

export enum ResponseType {
  response = 'response',
  event = 'event',
}

export type ResponseData = object;

export interface Response {
  uuid: string;
  type: ResponseType;
  success?: string;
  error?: string;
  data?: ResponseData;
}

export type Event = object;

export interface EventResponse extends Response {
  name: string;
  data: ResponseData;
}
