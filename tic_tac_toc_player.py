#!/opt/local/bin/python
# python equivalent of cpp player
import numpy as np
import sys
import random as rnd
from datetime import datetime  

EMPTY = 0
WHITE = 1
BLACK = 2

LOG_ACTIVE = 0

#
# quasi random tic tac toe player called tic tac toc :-)
# plays for direct victory if it is possible
# otherwise plays randomly
#
def init(strboard):
  ret = np.zeros((3, 3),dtype=int)
  for i in range(len(strboard)):
    col = i%3
    line = i//3
    if strboard[i] == 'o' :
      ret[line,col] = WHITE
    if strboard[i] == 'x' :
      ret[line,col] = BLACK
  return ret

def is_a_win(board, color):
  for i in range(3):
    if board[i,0]==color and board[i,1]==color and board[i,2]==color: 
      return True
  for i in range(3):
    if board[0,i]==color and board[1,i]==color and board[2,i]==color: 
      return True
  if board[0,0]==color and board[1,1]==color and board[2,2]==color: 
    return True
  if board[0,2]==color and board[1,1]==color and board[2,0]==color: 
    return True
  return False

def fprint_board(fd, board):
  print("[",board[0,0],",",end='', file=fd)
  for i in range(1,8):
    col = i%3
    line = i//3
    print(board[line,col],",",end='', file=fd)
  print(board[2,2],"]",file=fd)

def genmove(board, color):
  if LOG_ACTIVE:
    print("--- genmove", file=sys.stderr)
    print("board : ", end='', file=sys.stderr)
    fprint_board(sys.stderr,board)
    print("color : ", color, file=sys.stderr)
  nb_empty = len(np.where( board == 0)[0])
  r = rnd.randrange(nb_empty)
  for i in range(9):
    col = i%3
    line = i//3
    if board[line,col]==EMPTY:
      r=r-1
      if r==-1:
        if LOG_ACTIVE:
          print("--- return ", i, file=sys.stderr)
        return i      
  return -1

# to generate x move : python tic_tac_toc_player.py x.x...... x
# to generate o move : python tic_tac_toc_player.py x.x...... o
# returns where to play

def main(argc, argv):
  if argc != 3:
    print("usage: python ", argv[0], "STRBOARD TURN")
    return 0
  board = argv[1]
  turn = WHITE
  if(argv[2] == "x"):
    turn = BLACK  
  rnd.seed(datetime.now().timestamp())
  state = init(board)
  move = genmove(state,turn)
  print(move)
  return 0

if __name__ == '__main__':
  sys.exit(main(len(sys.argv),sys.argv))

