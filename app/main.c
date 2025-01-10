#include <stdio.h>
#include <string.h>

int main() {

  char input[100];

  do{
    // Flush after every printf
    setbuf(stdout, NULL);
    // Uncomment this block to pass the first stage
    printf("$ ");
    // Wait for user input
    fgets(input, 100, stdin);
    //Removes the new line of the input buffer
    input[strlen(input) - 1] = '\0';
    //Prints (input command): command not found
    printf("%s: command not found\n",input);
  }
  while(strcmp(input,"") != 0);
  
  return 0;
}
