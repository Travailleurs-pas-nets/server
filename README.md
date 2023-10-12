# server
The server for the chat application

## Communication contract
To communicate with the server (because any malformed request will be automatically thrown by the server), the client has to conform to the following requirements.

### Request shape
The request messages should be at most a 1024 characters array, composed of two distinct parts:
- the option code;
- the content.

The option code ranges from the first to until the twenty-fourth character (both included). The available options will be later discussed.

The content ranges from the twenty-fifth character until the end (1024 char max), and its value will vary according to the option code associated.

#### Option codes
| Code | Meaning                      | Expected content                                         |
|------|------------------------------|----------------------------------------------------------|
| `00` | Subscription to a discussion | The name of the channel the client wants to subscribe to |
| `01` | Sending a message            | The content of the message                               |

## Standard flow
When the server receives a request from a client to subscribe to a discussion, it will at first check if the discussion already exists.  
If the discussion is not found, it will be created, provided that the server capacity for discussions is not full yet.

The term subscription is all but anodyne in this context. It indicates the use of the **observer** pattern, on a wide scale. When a message is sent by a client in a discussion (that has to be the one the client is currently subscribed to), the server will follow this flow path:
- it will extract the message from the request content;
- and then send a request with the aforementioned content to all subscribed clients, except the one that sent the first request.
