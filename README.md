# Todo Cerver API Service

### Development
```
sudo docker run \
  -it \
  --name todo --rm \
  -p 5000:5000 \
  -v /home/ermiry/Documents/ermiry/Projects/todo-cerver:/home/todo \
  -e RUNTIME=development \
  -e PORT=5000 \
  -e CERVER_RECEIVE_BUFFER_SIZE=4096 -e CERVER_TH_THREADS=4 \
  -e CERVER_CONNECTION_QUEUE=4 \
  -e MONGO_APP_NAME=api -e MONGO_DB=todo \
  -e MONGO_URI=mongodb://api:password@192.168.100.39:27017/todo \
  -e PRIV_KEY=/home/todo/keys/key.key -e PUB_KEY=/home/todo/keys/key.pub \
  -e ENABLE_USERS_ROUTES=TRUE \
  ermiry/todo-cerver-api:development /bin/bash
```

## Routes

### Main

#### GET /api/todo
**Access:** Public \
**Description:** todo top level route \
**Returns:**
  - 200 on success

#### GET api/todo/version
**Access:** Public \
**Description:** Returns todo-api service current version \
**Returns:**
  - 200 and version's json on success

#### GET api/todo/auth
**Access:** Private \
**Description:** Used to test if jwt keys work correctly \
**Returns:**
  - 200 on success
  - 401 on failed auth

### Items

#### GET api/todo/items
**Access:** Private \
**Description:** Get all the authenticated user's todo items \
**Returns:**
  - 200 and items json on success
  - 401 on failed auth

#### POST api/todo/items
**Access:** Private \
**Description:** A user has requested to create a new item \
**Returns:**
  - 200 on success creating item
  - 400 on failed to create new item
  - 401 on failed auth
  - 500 on server error

#### GET api/todo/items/:id/info
**Access:** Private \
**Description:** Returns information about an existing item that belongs to a user \
**Returns:**
  - 200 and item's json on success
  - 401 on failed auth
  - 404 on item not found

#### PUT api/todo/items/:id/update
**Access:** Private \
**Description:** A user wants to update an existing item \
**Returns:**
  - 200 on success updating user's item
  - 400 on bad request due to missing values
  - 401 on failed auth
  - 500 on server error

#### DELETE api/todo/items/:id/remove
**Access:** Private \
**Description:** Deletes an existing user's item \
**Returns:**
  - 200 on success deleting user's item
  - 400 on bad request
  - 401 on failed auth
  - 500 on server error

### Users

#### GET /api/users
**Access:** Public \
**Description:** Users top level route \
**Returns:**
  - 200 on success

#### POST api/users/login
**Access:** Public \
**Description:** Uses the user's supplied creedentials to perform a login and generate a JWT \
**Returns:**
  - 200 and token on success authenticating user
  - 400 on bad request due to missing values
  - 404 on user not found
  - 500 on server error

#### POST api/users/register
**Access:** Public \
**Description:** Used by users to create a new account \
**Returns:**
  - 200 and token on success creating a new user
  - 400 on bad request due to missing values
  - 500 on server error
