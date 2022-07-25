#include "9cc.h"

void gen_prologue() {
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", locals->offset);
}

// 最後の式の結果がRAXに残っているのでそれが返り値になる
void gen_epilogue() {
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node *node, int depth) {
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs, depth);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
    case ND_RETURN:
      gen(node->lhs, depth);
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
    case ND_IF:
      if (node->els) {
        gen(node->cond, depth);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .L.else%d.%d\n", node->label, depth);
        gen(node->then, depth);
        printf("  jmp .L.endif%d.%d\n", node->label, depth);

        printf(".L.else%d.%d:\n", node->label, depth);
        gen(node->els, depth);

        printf(".L.endif%d.%d:\n", node->label, depth);
      } else {
        gen(node->cond, depth);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .L.endif%d.%d\n", node->label, depth);
        gen(node->then, depth);

        printf(".L.endif%d.%d:\n", node->label, depth);
      }
      return;
    case ND_FOR:
      if (node->init)
        gen(node->init, depth);
      printf(".L.beginloop%d.%d:\n", node->label, depth);
      if (node->cond) {
        gen(node->cond, depth);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .L.endloop%d.%d\n", node->label, depth);
      }
      gen(node->then, depth);
      if (node->inc)
        gen(node->inc, depth);
      printf("  jmp .L.beginloop%d.%d\n", node->label, depth);
      printf(".L.endloop%d.%d:\n", node->label, depth);
      return;
    case ND_BLOCK:
      for (Node *p = node->stmts; p; p = p->next) {
        gen(p->body, depth + 1);
        printf("  pop rax\n");
      }
      return;
  }

  gen(node->lhs, depth);
  gen(node->rhs, depth);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}
