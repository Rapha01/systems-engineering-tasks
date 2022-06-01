uint64_t* treiberStackRef;
uint64_t* getNextNode(uint64_t* node);
uint64_t getPayload(uint64_t* node);
void setNextNode(uint64_t* node, uint64_t* next);
void setPayload(uint64_t* node, uint64_t payload);
uint64_t* createNode();

void push(uint64_t value);
uint64_t pop();

void testTreiberStack();
uint64_t lastPopReturnValue;
uint64_t getTreiberStackSize();
void pushWithoutCAS(uint64_t value);

uint64_t main() {
  treiberStackRef = malloc(8);
  *treiberStackRef = 0;

  thread();
  thread();
  testTreiberStack();

  //return lastPopReturnValue();
  return getTreiberStackSize();
}

// TREIBERSTACK TEST

void testTreiberStack() {
  uint64_t i;

  i = 1;
  while(i < 7) {
    //pushWithoutCAS(i * 3 + 1);
    push(i * 3 + 1);
    i = i + 1;
  }

  i = 1;
  while(i < 3) {
    lastPopReturnValue = pop();
    i = i + 1;
  }
}


// TREIBER STACK

void push(uint64_t value) {
  uint64_t* newHead;
  uint64_t* oldHead;
  uint64_t success;

  newHead = createNode();
  setPayload(newHead, value);

  success = 0;
  while(success == 0) {
    oldHead = (uint64_t*) *treiberStackRef;
    setNextNode(newHead, oldHead);
    success = compare_and_swap((uint64_t) newHead, (uint64_t) oldHead, treiberStackRef);
  }
}

uint64_t pop() {
  uint64_t* newHead;
  uint64_t* oldHead;
  uint64_t success;

  success = 0;
  while(success == 0) {
    oldHead = (uint64_t*) *treiberStackRef;

    if(oldHead == (uint64_t*) 0) {
      return 0;
    }

    newHead = getNextNode(oldHead);
    success = compare_and_swap((uint64_t) newHead, (uint64_t) oldHead, treiberStackRef);
  }

  return getPayload(oldHead);
}

void pushWithoutCAS(uint64_t value) {
  uint64_t* newHead;
  uint64_t* oldHead;
  uint64_t success;

  newHead = createNode();
  setPayload(newHead, value);

  oldHead = (uint64_t*) *treiberStackRef;
  setNextNode(newHead, oldHead);
  *treiberStackRef = (uint64_t) newHead;
}

uint64_t getTreiberStackSize() {
  uint64_t* tempNode;
  uint64_t size;

  tempNode = (uint64_t*) *treiberStackRef;

  size = 0;
  while(tempNode != (uint64_t*) 0) {
    size = size + 1;
    tempNode = getNextNode(tempNode);
  }

  return size;
}


// Node Struct

// +----+----------------+
// |  0 | nextNode       | pointer to next Node
// |  1 | payload        | value
// +----+----------------+

uint64_t* getNextNode(uint64_t* node)               { return (uint64_t*) *node; }
uint64_t getPayload(uint64_t* node)                 { return *(node + 1); }

void setNextNode(uint64_t* node, uint64_t* next)    { *node          = (uint64_t) next; }
void setPayload(uint64_t* node, uint64_t payload)   { *(node + 1)    = payload; }

uint64_t* createNode() {
  uint64_t* newNode;

  newNode = malloc(16);
  setNextNode(newNode, (uint64_t*) 0);
  setPayload(newNode, 0);

  return newNode;
}
