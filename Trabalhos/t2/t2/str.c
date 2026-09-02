// includes, constantes e declarações {{{1
#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MIN_ALLOC 8    // alocação mínima

struct str {
  byte *bytes;
  int n_bytes;
  int n_chars;
  int capacidadeMax;
};

// A memória para conter os bytes de uma string deve ser alocada e/ou
//    realocada conforme a necessidade, cuidando para que a quantidade
//    de memória alocada seja sempre:
//    - nula (não alocada) se a string for vazia, ou
//    - não inferior ao necessário para armazenar os bytes da codificação utf8;
//    - não inferior à alocação mínima;
//    - não superior ao triplo do número de bytes necessários
//      (exceto quando for o mínimo);
//    - uma potência de 2.

// funções auxiliares {{{1

static int calcula_tam(int n) {
  if (n <= MIN_ALLOC) return MIN_ALLOC;
  int p = MIN_ALLOC;
  while (p < n) {
    p *= 2;
  }
  return p;
}

static void realoca(Str s, int bytesNecessarios) {
  if (bytesNecessarios == 0) {
    if (s->bytes != NULL) {
      free(s->bytes);
      s->bytes = NULL;
    }
    s->capacidadeMax = 0;
    return;
  }

  if (s->capacidadeMax < bytesNecessarios || s->capacidadeMax > 3 * bytesNecessarios) {
    int nova_cap = calcula_tam(bytesNecessarios);
    s->bytes = realloc(s->bytes, nova_cap);
    assert(s->bytes != NULL);
    s->capacidadeMax = nova_cap;
  }
}

// verifica se a string cad está de acordo com a especificação
// aborta o programa se não tiver
static void s_ok(Str_c s)
{
  assert(s != NULL);
  if (s->n_bytes == 0) {
    assert(s->bytes == NULL);
    assert(s->capacidadeMax == 0);
    assert(s->n_chars == 0);
  } else {
    assert(s->bytes != NULL);
    assert(s->capacidadeMax >= s->n_bytes);
    assert(s->capacidadeMax >= MIN_ALLOC);
    assert(s->capacidadeMax <= 3 * s->n_bytes || s->capacidadeMax == MIN_ALLOC);
    assert((s->capacidadeMax & (s->capacidadeMax - 1)) == 0);
    assert(s->n_chars == u8_conta_unichar_nos_bytes(s->n_bytes, s->bytes));
  }
}

static int normaliza_pos(int pos, int n_chars) {
  if (pos < 0) {
    pos = n_chars + 1 + pos;
  }
  if (pos < 0) pos = 0;
  if (pos > n_chars) pos = n_chars;
  return pos;
}

static int char_para_byte(Str_c s, int pos_char) {
  if (pos_char <= 0) return 0;
  if (pos_char >= s->n_chars) return s->n_bytes;

  byte *ptr = u8_avanca_unichar(s->bytes, pos_char);
  if (ptr == NULL) return s->n_bytes;
  return (int)(ptr - s->bytes);
}

// operações de criação e destruição {{{1

Str s_cria(char const *strC)
{
  Str s = malloc(sizeof(*s));
  assert(s != NULL);
  s->bytes = NULL;
  s->n_bytes = 0;
  s->n_chars = 0;
  s->capacidadeMax = 0;
  if (strC == NULL) {
    s_ok(s);
    return s;
  }
  int len = strlen(strC);
  if (len == 0) {
    s_ok(s);
    return s;
  }

  int n_chars = u8_conta_unichar_nos_bytes(len, (byte *)strC);
  if (n_chars < 0) {
    s_ok(s);
    return s;
  }

  realoca(s, len);
  memcpy(s->bytes, strC, len);
  s->n_bytes = len;
  s->n_chars = n_chars;

  s_ok(s);
  return s;
}

void s_destroi(Str s)
{
  if (s == NULL) return;
  s_ok(s);
  if (s->bytes != NULL) {
    free(s->bytes);
  }
  free(s);
}

Str s_cria_substring(Str_c s, int pos, int tam)
{
   Str nova = s_cria("");
   s_substring(nova, s, pos, tam);
   return nova;
}

Str s_cria_cópia(Str_c s)
{
   return s_cria_substring(s, 0, -1);
}

// Retorna uma nova string com o conteúdo do arquivo chamado nome.
// Retorna uma string vazia em caso de erro.
Str s_cria_de_arquivo(char *nome)
{
  Str s = s_cria("");
  if (nome == NULL) return s;

  FILE *f = fopen(nome, "rb");
  if (!f) return s;

  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fseek(f, 0, SEEK_SET);

  if (len <= 0) {
    fclose(f);
    return s;
  }

  byte *buf = malloc(len);
  assert(buf != NULL);

  size_t lidos = fread(buf, 1, len, f);
  fclose(f);

  if (lidos == (size_t)len) {
    int n_chars = u8_conta_unichar_nos_bytes((int)len, buf);
    if (n_chars >= 0) {
      realoca(s, (int)len);
      memcpy(s->bytes, buf, len);
      s->n_bytes = (int)len;
      s->n_chars = n_chars;
    }
  }

  free(buf);
  s_ok(s);
  return s;
}

// operações de acesso {{{1

int s_tam(Str_c s)
{
  s_ok(s);
  return s->n_chars;
}

char *s_strc(Str_c s)
{
  s_ok(s);
  char *str = malloc(s->n_bytes + 1);
  assert(str != NULL);
  if (s->n_bytes > 0) {
    memcpy(str, s->bytes, s->n_bytes);
  }
  str[s->n_bytes] = '\0';
  return str;
}

unichar s_ch(Str_c s, int pos)
{
  s_ok(s);

  int real_pos = pos;
  if (real_pos < 0) {
    real_pos = s->n_chars + 1 + real_pos;
  }

  if (real_pos < 0 || real_pos >= s->n_chars) {
    return UNI_INV;
  }

  int b_idx = char_para_byte(s, real_pos);
  unichar c;
  if (u8_unichar_nos_bytes(s->n_bytes - b_idx, s->bytes + b_idx, &c) < 1) {
    return UNI_INV;
  }
  return c;
}


// operações de busca e comparação {{{1

bool s_igual(Str_c s, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  if (s->n_bytes != sb->n_bytes) return false;
  if (s->n_bytes == 0) return true;
  return memcmp(s->bytes, sb->bytes, s->n_bytes) == 0;
}

int s_busca_c(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  int p = normaliza_pos(pos, s->n_chars);

  for (int i = p; i < s->n_chars; i++) {
    unichar c = s_ch(s, i);
    for (int j = 0; j < sb->n_chars; j++) {
      if (c == s_ch(sb, j)) {
        return i;
      }
    }
  }
  return -1;
}

int s_busca_nc(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  int p = normaliza_pos(pos, s->n_chars);

  for (int i = p; i < s->n_chars; i++) {
    unichar c = s_ch(s, i);
    bool achou = false;
    for (int j = 0; j < sb->n_chars; j++) {
      if (c == s_ch(sb, j)) {
        achou = true;
        break;
      }
    }
    if (!achou) return i;
  }
  return -1;
}

int s_busca_rc(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  int p = normaliza_pos(pos, s->n_chars);

  for (int i = p - 1; i >= 0; i--) {
    unichar c = s_ch(s, i);
    for (int j = 0; j < sb->n_chars; j++) {
      if (c == s_ch(sb, j)) {
        return i;
      }
    }
  }
  return -1;
}

int s_busca_rnc(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  int p = normaliza_pos(pos, s->n_chars);

  for (int i = p - 1; i >= 0; i--) {
    unichar c = s_ch(s, i);
    bool achou = false;
    for (int j = 0; j < sb->n_chars; j++) {
      if (c == s_ch(sb, j)) {
        achou = true;
        break;
      }
    }
    if (!achou) return i;
  }
  return -1;
}

int s_busca_s(Str_c s, int pos, Str_c buscada)
{
  s_ok(s);
  s_ok(buscada);
  int p = normaliza_pos(pos, s->n_chars);

  if (buscada->n_chars == 0) {
    return p;
  }

  for (int i = p; i <= s->n_chars - buscada->n_chars; i++) {
    bool bateu = true;
    for (int j = 0; j < buscada->n_chars; j++) {
      if (s_ch(s, i + j) != s_ch(buscada, j)) {
        bateu = false;
        break;
      }
    }
    if (bateu) return i;
  }
  return -1;
}


// operações de alteração {{{1

void s_substitui(Str s, int pos, int tam, Str_c sb)
{
  s_ok(s);
  if (sb != NULL) s_ok(sb);

  int p_ini = normaliza_pos(pos, s->n_chars);
  int p_fim;

  if (tam < 0) {
    p_fim = s->n_chars;
  } else {
    p_fim = p_ini + tam;
    if (p_fim > s->n_chars) p_fim = s->n_chars;
  }

  int b_ini = char_para_byte(s, p_ini);
  int b_fim = char_para_byte(s, p_fim);
  int b_rem = b_fim - b_ini;

  int b_add = (sb != NULL) ? sb->n_bytes : 0;
  int novos_bytes = s->n_bytes - b_rem + b_add;

  if (novos_bytes == 0) {
    realoca(s, 0);
    s->n_bytes = 0;
    s->n_chars = 0;
    s_ok(s);
    return;
  }

  if (b_add > b_rem) {
    realoca(s, novos_bytes);
  }

  int cauda_bytes = s->n_bytes - b_fim;
  if (cauda_bytes > 0) {
    memmove(s->bytes + b_ini + b_add, s->bytes + b_fim, cauda_bytes);
  }

  if (b_add > 0) {
    memcpy(s->bytes + b_ini, sb->bytes, b_add);
  }

  s->n_bytes = novos_bytes;
  if (b_add < b_rem) {
    realoca(s, novos_bytes);
  }

  s->n_chars = u8_conta_unichar_nos_bytes(s->n_bytes, s->bytes);
  s_ok(s);
}

void s_substring(Str s, Str_c sb, int pos, int tam)
{
  s_ok(s);
  s_ok(sb);

  if (s == sb) {
    Str temp = s_cria_cópia(sb);
    s_substring(s, temp, pos, tam);
    s_destroi(temp);
    return;
  }

  int p_ini = normaliza_pos(pos, sb->n_chars);
  int p_fim;

  if (tam < 0) {
    p_fim = sb->n_chars;
  } else {
    p_fim = p_ini + tam;
    if (p_fim > sb->n_chars) p_fim = sb->n_chars;
  }

  if (p_ini >= p_fim) {
    realoca(s, 0);
    s->n_bytes = 0;
    s->n_chars = 0;
    s_ok(s);
    return;
  }

  int b_ini = char_para_byte(sb, p_ini);
  int b_fim = char_para_byte(sb, p_fim);
  int novos_bytes = b_fim - b_ini;

  realoca(s, novos_bytes);
  memmove(s->bytes, sb->bytes + b_ini, novos_bytes);
  s->n_bytes = novos_bytes;

  if (s->capacidadeMax > 3 * novos_bytes) {
    realoca(s, novos_bytes);
  }

  s->n_chars = u8_conta_unichar_nos_bytes(s->n_bytes, s->bytes);
  s_ok(s);
}

void s_copia(Str s, Str_c sb)
{
  s_substring(s, sb, 0, -1);
}

void s_insere(Str s, int pos, Str_c sb)
{
  s_substitui(s, pos, 0, sb);
}

void s_insere_c(Str s, int pos, unichar c)
{
  s_ok(s);
  byte buf[4];
  int n_bytes = u8_converte_pra_utf8(c, buf);
  if (n_bytes <= 0) return;

  struct str aux;
  aux.bytes = buf;
  aux.n_bytes = n_bytes;
  aux.n_chars = 1;
  aux.capacidadeMax = MIN_ALLOC;

  s_substitui(s, pos, 0, &aux);
}

void s_anexa(Str s, Str_c sb)
{
  s_substitui(s, -1, 0, sb);
}

void s_anexa_c(Str s, unichar c)
{
  s_insere_c(s, -1, c);
}

void s_remove(Str s, int pos, int tam)
{
  s_substitui(s, pos, tam, NULL);
}

void s_apara(Str s, Str_c sobras)
{
  s_ok(s);
  s_ok(sobras);

  if (s->n_chars == 0) return;

  int inicio = s_busca_nc(s, 0, sobras);
  if (inicio == -1) {
    s_substitui(s, 0, -1, NULL);
    return;
  }

  int fim = s_busca_rnc(s, -1, sobras);

  if (fim < s->n_chars - 1) {
    s_remove(s, fim + 1, -1);
  }
  if (inicio > 0) {
    s_remove(s, 0, inicio);
  }
}

// operações de E/S {{{1

void s_imprime(Str_c s)
{
  s_ok(s);
  if (s->n_bytes > 0) {
    fwrite(s->bytes, 1, s->n_bytes, stdout);
  }
}

void s_grava_arquivo(Str_c s, char *nome)
{
  s_ok(s);
  if (nome == NULL) return;

  FILE *f = fopen(nome, "wb");
  if (!f) return;

  if (s->n_bytes > 0) {
    fwrite(s->bytes, 1, s->n_bytes, f);
  }
  fclose(f);
}
// vim: foldmethod=marker shiftwidth=2
