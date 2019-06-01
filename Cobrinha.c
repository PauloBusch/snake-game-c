#define _WIN32_WINNT 0x0500 

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <stdbool.h>
#include <windows.h>
#include<dos.h>

//========== CONSTANTES =========//
#define OBS_RAIZES 30
#define OBS_ROWS 30
#define OBS_BLOK 5
#define OFFSET_X 2

#define EMPTY_ROW ' '
#define VALID_ROW '+'
#define BORDA_ROW 177
#define OBST_ROW  176

#define CABEC_COB 178
#define BODY_COB  178
#define MACA_CHAR '@'

#define SETAS     0xE0
#define KEY_UP    0x48
#define KEY_DOWN  0x50
#define KEY_LEFT  0x4B
#define KEY_RIGTH 0x4D

#define D_TOP direcao_cima()
#define D_DOW direcao_baixo()
#define D_LEF direcao_esquerda()
#define D_RHT direcao_direita()

//============= TAD's ============//
typedef struct ROW{
    char elm;

    int pos_x;
    int pos_y;

    struct ROW *prox;
} ROW;

typedef struct DIRECAO{
    int inc_x;
    int inc_y;
} DIRECAO;

typedef struct COBRA{
    DIRECAO *direcao;
    ROW *final;
    ROW *cabec;
} COBRA;

typedef struct MACA{
    ROW *row;
} MACA;

typedef struct No{
    void *val;

    struct No *prox;
} No;

typedef struct FILA{
    No *ini;
    No *fim;
} FILA;

//=========== VARIÁVEIS ==========//
int DIMENSAO_X, DIMENSAO_Y;

//========== ASSINATURAS =========//
COBRA   *cria_cobra();
MACA    *cria_maca();
ROW     *cria_row();
FILA    *cria_fila();
char    **cria_matriz();
DIRECAO *cria_direcao();

void imprime_cobra(COBRA *cobra);
void imprime_maca(MACA *maca);
void imprime_borda(char **matriz);
void imprime_obstaculo(char **matriz);
void imprime_mensagem(char *mensagem);

void atualiza_direcao(int tecla, DIRECAO *direcao);
void atualiza_cobra(COBRA *cobra);
void atualiza_posicao(ROW *row, DIRECAO *direcao);
void atualiza_maca(COBRA *cobra, char **matriz);
void atualiza_area_trabalho(char **matriz, COBRA *cobra);

char verifica_obstaculo(ROW *row, DIRECAO *direcao);
int  verifica_barreiras(ROW *row, char **matriz);
bool verifica_colisao(char elm);
bool verifica_maca(char elm);
bool verifica_cobra(char elm);
bool verifica_fila_vazia(FILA *fila);

char **copiar_matriz(char **b_matriz);
ROW *copiar_row(ROW *b_row);

void libera_matriz(char **b_matriz);
void libera_cobra(COBRA *cobra);
void libera_maca(MACA *maca);
void libera_fila(FILA *fila);
void libera_direcao(DIRECAO *direcao);

void alimenta_cobra(COBRA *cobra);
void alimenta_fila(FILA *fila, void *val);

DIRECAO *direcao_cima();
DIRECAO *direcao_baixo();
DIRECAO *direcao_esquerda();
DIRECAO *direcao_direita();

ROW     *sorteia_row(DIRECAO *direcao);
ROW     **sorteia_obstaculos();
MACA    *sorteia_maca();
DIRECAO *sorteia_direcao();

void set_char_by_cursor(char c,int x,int y);
char get_char_by_cursor(int x,int y);
void get_size_window(int *col, int *row);
void hide_cursor(bool hide);
void run_cursor();
void maximize_window();

//============== MAIN ============//
int main(){
    
    //Iniciando tela
    maximize_window();
    get_size_window(&DIMENSAO_X, &DIMENSAO_Y);
    run_cursor();

    //Iniciando jogo
    bool jogando = true;
    do{
        //Iniciando variáveis
        srand(time(NULL));
        char **quadro = cria_matriz();
        COBRA *cobra  = cria_cobra();
        MACA *maca    = cria_maca();
        bool vivo = true;

        //Desenhando componentes
        system("cls");
        imprime_borda(quadro);
        //TODO: sortear no meio
        imprime_maca(maca);
        imprime_cobra(cobra);
        //TODO: colocar pra cima
        imprime_obstaculo(quadro);
        atualiza_area_trabalho(quadro, cobra);

        //Atualização de frames    
        int tecla = getch();
        do{           
            if(tecla == SETAS){  //Código das setas
                tecla = getch(); //Direção da seta

                atualiza_direcao(tecla, cobra->direcao);
            }
        
            char elm_coli = verifica_obstaculo(
                cobra->cabec, 
                cobra->direcao);

            if(verifica_colisao(elm_coli)){
                hide_cursor(false);
                imprime_mensagem("VOCE COLIDIU!, deseja reiniciar? [s/n] ");
                scanf("%c",&tecla);
                while(tecla != 's' && tecla != 'n'){
                    imprime_mensagem("TECLA INCORRETA!, deseja reiniciar? [s/n] ");
                    scanf("%c",&tecla);
                }
                hide_cursor(true);
                jogando = tecla == 's';
                vivo = false;
                break;
            }

            if(verifica_maca(elm_coli)){
                alimenta_cobra(cobra);
                atualiza_maca(cobra, quadro);
            }

            atualiza_cobra(cobra);
            Sleep(100);        

            if(kbhit())
                tecla = getch();
        }while(vivo);

        //libera_matriz(quadro);
        libera_cobra(cobra);
        libera_maca(maca);
    }while(jogando);

        
    hide_cursor(false);
    imprime_mensagem("FIM DE JOGO!, tecle qualquer tecla para fechar...");
    getch();

    return 0;
}

//====== ALOCACAO DE TAD's ========//
COBRA *cria_cobra(){
    
    int pos_x = (int)(DIMENSAO_X / 3);

    COBRA *cobra = (COBRA*) malloc(sizeof(COBRA));
    cobra->cabec = (ROW*) malloc(sizeof(ROW));
    cobra->cabec->elm = CABEC_COB;
    cobra->cabec->prox = NULL;
    cobra->cabec->pos_x = pos_x - pos_x % OFFSET_X; 
    cobra->cabec->pos_y = (int)(DIMENSAO_Y / 2);
    cobra->final = cobra->cabec;
    cobra->direcao = cria_direcao();
    cobra->direcao->inc_x = OFFSET_X;
    cobra->direcao->inc_y = 0;

    return cobra;
}

MACA *cria_maca(){
    
    int pos_x = (int)(DIMENSAO_X / 2);

    MACA *maca = (MACA*) malloc(sizeof(MACA));
    maca->row = (ROW*) malloc(sizeof(ROW));
    maca->row->elm = MACA_CHAR;
    maca->row->prox = NULL;
    maca->row->pos_x = pos_x - pos_x % OFFSET_X; 
    maca->row->pos_y = (int)(DIMENSAO_Y / 2);

    return maca;
}

char **cria_matriz(){

    char **m = (char**) malloc(sizeof(char*) * DIMENSAO_X + OFFSET_X);

    int idx_x, idx_y;
    for(idx_x = 0; idx_x < DIMENSAO_X + OFFSET_X; idx_x++){
        m[idx_x] = (char*) malloc(sizeof(char) * DIMENSAO_Y);
        for(idx_y = 0; idx_y < DIMENSAO_Y; idx_y++){
            m[idx_x][idx_y] = EMPTY_ROW;
        }
    }

    return m;
}

DIRECAO *cria_direcao(){

    DIRECAO *direcao = (DIRECAO*)malloc(sizeof(DIRECAO));

    direcao->inc_x = 0;
    direcao->inc_y = 0;

    return direcao;
}

ROW *cria_row(){
    ROW *r = (ROW*) malloc(sizeof(ROW));
    
    r->prox = NULL;

    return r;
}

FILA *cria_fila(){
    FILA *fila = (FILA*) malloc(sizeof(FILA));
    
    fila->ini = NULL;
    fila->fim = NULL;

    return fila;
}

//============= DESENHOS ============//
void imprime_cobra(COBRA *cobra){

    ROW *r = cobra->final;
    
    while(r != NULL){
        set_char_by_cursor(r->elm,r->pos_x,r->pos_y);
        
        r = r->prox;
    }
}

void imprime_maca(MACA *maca){

    ROW * r = maca->row;

    set_char_by_cursor(r->elm,r->pos_x,r->pos_y);
}

void imprime_borda(char **matriz){

    //Borda X
    int idx_x, idx_y, off_x, dmi_x = DIMENSAO_X - DIMENSAO_X % OFFSET_X;
    for(idx_x = 0; idx_x < dmi_x; idx_x++){
        matriz[idx_x][0            ] = BORDA_ROW;
        matriz[idx_x][DIMENSAO_Y -1] = BORDA_ROW;

        set_char_by_cursor(BORDA_ROW,idx_x,0);
        set_char_by_cursor(BORDA_ROW,idx_x, DIMENSAO_Y -1);
    }

    //Borda Y   
    for(idx_y = 0; idx_y < DIMENSAO_Y; idx_y++){
        for(off_x = 0; off_x < OFFSET_X; off_x++){
            matriz[off_x          ][idx_y] = BORDA_ROW;
            matriz[dmi_x -off_x -1][idx_y] = BORDA_ROW;

            set_char_by_cursor(BORDA_ROW,off_x,idx_y);
            set_char_by_cursor(BORDA_ROW,dmi_x - off_x -1, idx_y);
        }
    }

}

void imprime_obstaculo(char **matriz){

    ROW **obstaculos = sorteia_obstaculos();

    int idx_obs, idx_offset;
    for(idx_obs = 0; idx_obs < OBS_RAIZES; idx_obs++){
        ROW *obs = obstaculos[idx_obs];

        while(obs != NULL){
    
            //Iprime nos offsets
            for(idx_offset = 0; idx_offset < OFFSET_X; idx_offset++){
                matriz[obs->pos_x + idx_offset][obs->pos_y] = obs->elm;

                set_char_by_cursor(obs->elm,obs->pos_x + idx_offset, obs->pos_y);
            }

            obs = obs->prox;
        }
    }
}

void imprime_mensagem(char *mensagem){
    
    int idx_x, idx_y, idx_offset, size = 1;

    //Calcula tamanho da string
    while(mensagem[size] != '\0')
        size++;

    int div_y = DIMENSAO_Y / 2, s_x = (DIMENSAO_X / 2) - (size / 2);

    //Borda X
    for(idx_x = -2; idx_x < size +2; idx_x++){

        //Iprime nos offsets
        for(idx_offset = 0; idx_offset < OFFSET_X; idx_offset++){
            int pos_x = s_x + idx_x + idx_offset;

            //Borda externa
            set_char_by_cursor(178,pos_x, div_y -2);
            set_char_by_cursor(178,pos_x, div_y +2);

            //Borda interna
            set_char_by_cursor(EMPTY_ROW,pos_x, div_y -1);
            set_char_by_cursor(EMPTY_ROW,pos_x, div_y);
            set_char_by_cursor(EMPTY_ROW,pos_x, div_y +1);
        }
    }

    //Mensagem    
    set_char_by_cursor(EMPTY_ROW,s_x, div_y);
    puts(mensagem);
    set_char_by_cursor(EMPTY_ROW,s_x + size, div_y);
}

//=========== ATUALIZAÇÕES ==========//
void atualiza_cobra(COBRA *cobra){

    ROW *fim = cobra->final;
    ROW *cab = cobra->cabec;
    int idx_offset;

    for(idx_offset = 0; idx_offset < OFFSET_X; idx_offset++){
        //Define corpo onde está a cabeça
        set_char_by_cursor(BODY_COB,cab->pos_x + idx_offset, cab->pos_y);

        //Limpa ultima posicao
        set_char_by_cursor(EMPTY_ROW,fim->pos_x + idx_offset, fim->pos_y);
    }

    while(fim != NULL){
        
        if(fim->prox != NULL){
            fim->pos_x = fim->prox->pos_x;
            fim->pos_y = fim->prox->pos_y;
        }

        fim = fim->prox;
    }

    //Atualiza e imprime cabeça
    cab->pos_x += cobra->direcao->inc_x;
    cab->pos_y += cobra->direcao->inc_y;

    for(idx_offset = 0; idx_offset < OFFSET_X; idx_offset++){
        set_char_by_cursor(cab->elm,cab->pos_x + idx_offset, cab->pos_y);
    }

}

void atualiza_direcao(int tecla, DIRECAO *direcao){

    switch(tecla){ 
        case KEY_UP:
            if(direcao->inc_y > 0)
                return;
            direcao->inc_x = 0;
            direcao->inc_y =-1;
            break;
        case KEY_DOWN:
            if(direcao->inc_y < 0)
                return;
            direcao->inc_x = 0;
            direcao->inc_y = 1;
            break;
        case KEY_LEFT:
            if(direcao->inc_x > 0)
                return;
            direcao->inc_x =-OFFSET_X;
            direcao->inc_y = 0;
            break;
        case KEY_RIGTH:
            if(direcao->inc_x > 0)
                return;
            direcao->inc_x = OFFSET_X;
            direcao->inc_y = 0;
            break;
    }
}

void atualiza_posicao(ROW *row, DIRECAO *direcao){
    row->pos_x += direcao->inc_x;
    row->pos_y += direcao->inc_y;
}

void atualiza_maca(COBRA *cobra, char **matriz){    

    //Regras de aceitação
    MACA *maca;
    int p_x, p_y;
    char elm;

    do{
        maca = sorteia_maca();
        elm = matriz[maca->row->pos_x][maca->row->pos_y];
    }while(elm != VALID_ROW || verifica_barreiras(maca->row,matriz) >= 3);

    imprime_maca(maca);
}

void atualiza_area_trabalho(char **matriz, COBRA *cobra){    
    FILA *level_in;
    FILA *level_ex;

    level_in = cria_fila();
    alimenta_fila(level_in, copiar_row(cobra->cabec));//Raiz
    
    No *no;
    ROW *row;
    do{
        level_ex = cria_fila();

        no = level_in->ini;
        while(no != NULL){
            row = (ROW*) no->val;

            row->prox = (ROW*) malloc(sizeof(ROW) * 4);
            
            int idx_i;
            char elm;
            ROW *dir;
            for(idx_i = 0; idx_i < 4; idx_i++){
                dir = &row->prox[idx_i];

                dir->pos_x = row->pos_x;
                dir->pos_y = row->pos_y;

                if(idx_i == 0)atualiza_posicao(dir, D_DOW );
                if(idx_i == 1)atualiza_posicao(dir, D_TOP );
                if(idx_i == 2)atualiza_posicao(dir, D_LEF );
                if(idx_i == 3)atualiza_posicao(dir, D_RHT );
                
                elm = matriz[dir->pos_x][dir->pos_y];

                if(elm == EMPTY_ROW){
                    //set_char_by_cursor(VALID_ROW,dir->pos_x,dir->pos_y);
                    matriz[dir->pos_x][dir->pos_y] = VALID_ROW;
                    alimenta_fila(level_ex, dir);
                }
            }

            //free(rows);
            no = no->prox;
        }        
        //libera_fila(level_in->prox);
        level_in = level_ex;
    }while(!verifica_fila_vazia(level_ex));

    // libera_fila(level_in);
    // libera_fila(level_ex);
}

//========= ELEMENTOS =========//
void alimenta_cobra(COBRA *cobra){
    
    ROW *fim = cobra->final;
    ROW *novo = (ROW*) malloc(sizeof(ROW));

    novo->pos_x = fim->pos_x;
    novo->pos_y = fim->pos_y;
    novo->elm = BODY_COB;
    novo->prox = fim;

    cobra->final = novo;
}

void alimenta_fila(FILA *fila, void *val){
    No *no = (No*) malloc(sizeof(No));

    no->val = val;
    no->prox = NULL;
    
    if(verifica_fila_vazia(fila)){
        fila->ini = no;
        fila->fim = no;
    }else{
        fila->fim->prox = no;
        fila->fim = no;
    }
}

//======== VALIDAÇÕES =========//
char verifica_obstaculo(ROW *row, DIRECAO *direcao){
        
    int new_pos_x = row->pos_x + direcao->inc_x;
    int new_pos_y = row->pos_y + direcao->inc_y;

    return get_char_by_cursor(new_pos_x, new_pos_y);
}

int  verifica_barreiras(ROW *row, char **matriz){
    int barreiras = 0;
    if(matriz[row->pos_x + D_LEF->inc_x][row->pos_y + D_LEF->inc_y] != VALID_ROW)
        barreiras++;
    if(matriz[row->pos_x + D_RHT->inc_x][row->pos_y + D_RHT->inc_y] != VALID_ROW)
        barreiras++;
    if(matriz[row->pos_x + D_TOP->inc_x][row->pos_y + D_TOP->inc_y] != VALID_ROW)
        barreiras++;
    if(matriz[row->pos_x + D_DOW->inc_x][row->pos_y + D_DOW->inc_y] != VALID_ROW)
        barreiras++;  

    return barreiras;   
}

bool verifica_colisao(char elm){
    return (elm != EMPTY_ROW && elm != MACA_CHAR);
}

bool verifica_maca(char elm){
    return (elm == MACA_CHAR);
}

bool verifica_cobra(char elm){
    return (elm == (char)CABEC_COB || elm == (char)BODY_COB);
}

bool verifica_fila_vazia(FILA *fila){
    return (fila->ini == NULL && fila->fim == NULL);
}

//============== CÓPIAS =============//
char **copiar_matriz(char **b_matriz){
    char **n_matriz = (char **) malloc(sizeof(char*) * DIMENSAO_X);

    int idx_x, idx_y;
    for(idx_x = 0; idx_x < DIMENSAO_X; idx_x++){
        n_matriz[idx_x] = (char *) malloc(sizeof(char) * DIMENSAO_Y);
        for(idx_y = 0; idx_y < DIMENSAO_Y; idx_y++){
            n_matriz[idx_x][idx_y] = b_matriz[idx_x][idx_y];
        }
    }
    return n_matriz;
}

ROW *copiar_row(ROW *b_row){
    ROW *r = cria_row();

    r->pos_x = b_row->pos_x;
    r->pos_y = b_row->pos_y;

    return r;
}

//========== LIBERANDO =========//
void libera_matriz(char **b_matriz){
    int idx_x;
    for(idx_x = 0; idx_x < DIMENSAO_X + OFFSET_X; idx_x++){
        free(b_matriz[idx_x]);
    }
    free(b_matriz);
}

void libera_cobra(COBRA *cobra){
    ROW *no = cobra->final;
    ROW *an;

    while(no != NULL){
        an = no;
        no = no->prox;
        free(an);
    }
}

void libera_maca(MACA *maca){
    free(maca);
}

void libera_fila(FILA *fila){
    No *no = fila->ini;
    No *an;

    while(no != NULL){
        an = no;
        no = no->prox;
        free(an);
    }
}

void libera_direcao(DIRECAO *direcao){
    free(direcao);
}

//========= DIREÇÕES =========//
DIRECAO *direcao_cima(){
    DIRECAO *dir = cria_direcao();
    atualiza_direcao(KEY_UP, dir);
    return dir;
}
DIRECAO *direcao_baixo(){
    DIRECAO *dir = cria_direcao();
    atualiza_direcao(KEY_DOWN, dir);
    return dir;
}
DIRECAO *direcao_esquerda(){
    DIRECAO *dir = cria_direcao();
    atualiza_direcao(KEY_LEFT, dir);
    return dir;
}
DIRECAO *direcao_direita(){
    DIRECAO *dir = cria_direcao();
    atualiza_direcao(KEY_RIGTH, dir);
    return dir;
}

//========== SORTEIOS ==========//
MACA *sorteia_maca(){

    MACA *maca = cria_maca();
    DIRECAO *direcao = cria_direcao();
    maca->row = sorteia_row(direcao);
    maca->row->elm = MACA_CHAR;

    return maca;
}

DIRECAO *sorteia_direcao(){

    DIRECAO *direcao = cria_direcao();
    int direcoes[] = { KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGTH };
    int idx_rnd = rand() % 4;
    atualiza_direcao(direcoes[idx_rnd], direcao);
    
    return direcao;
}

ROW *sorteia_row(DIRECAO *direcao){
    ROW *row = (ROW*) malloc(sizeof(ROW));
    row->prox = NULL;

    do{
        row->pos_x = rand() % DIMENSAO_X;
        row->pos_y = rand() % DIMENSAO_Y;
        row->pos_x = row->pos_x - row->pos_x % OFFSET_X;
    }while(verifica_obstaculo(row, direcao) != EMPTY_ROW);

    return row;
}

ROW **sorteia_obstaculos(){
    
    ROW **obstaculos = (ROW**) malloc(sizeof(ROW*) * OBS_RAIZES);
    int idx_raiz = 0, idx_row = 0;
    DIRECAO *direcao = sorteia_direcao();

    while(idx_raiz < OBS_RAIZES){
        obstaculos[idx_raiz] = sorteia_row(cria_direcao());
        obstaculos[idx_raiz]->elm = OBST_ROW;
        ROW *obstaculo = obstaculos[idx_raiz];

        idx_row = 1;
        while(idx_row < OBS_ROWS){
            //Alterna direção
            if((idx_row % OBS_BLOK +1) == OBS_BLOK)
                direcao = sorteia_direcao();

            //Se bateu para
            if(verifica_obstaculo(obstaculo, direcao) != EMPTY_ROW)
                break;

            //Novo obstáculo
            ROW *novo = (ROW*) malloc(sizeof(ROW));

            novo->pos_x = obstaculo->pos_x;
            novo->pos_y = obstaculo->pos_y;
            novo->elm = OBST_ROW;
            novo->prox = NULL;
            
            atualiza_posicao(novo, direcao);

            obstaculo->prox = novo;
            obstaculo = novo;

            idx_row++;
        }

        idx_raiz++;
    }

    return obstaculos;
}

//=========== CONSOLE ==========//
void run_cursor(){
    hide_cursor(true);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD pos = {DIMENSAO_X,DIMENSAO_Y};
    DWORD esc;
    DWORD tam = csbi.dwSize.X *csbi.dwSize.Y;
    FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE),EMPTY_ROW,tam,pos,&esc);
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),pos);
}

void set_char_by_cursor(char c,int x,int y){
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),coord);
    
    putchar(c);
}

char get_char_by_cursor(int x,int y){
    char buf[2]; 
    COORD coord = {x,y}; 
    DWORD num_read;
	HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE);
	ReadConsoleOutputCharacter(hStd,(LPTSTR)buf,1,coord,(LPDWORD)&num_read);
	return buf[0];
}

void hide_cursor(bool hide)
{
   HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
   CONSOLE_CURSOR_INFO info;
   info.dwSize = 100;
   info.bVisible = !hide;
   SetConsoleCursorInfo(consoleHandle, &info);
}

void maximize_window()
{
    HWND consoleWindow = GetConsoleWindow();
    ShowWindow(consoleWindow, SW_MAXIMIZE);
}

void get_size_window(int *col, int *row){
    CONSOLE_SCREEN_BUFFER_INFO cmd;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cmd);
    *col = cmd.srWindow.Right - cmd.srWindow.Left +1;
    *row = cmd.srWindow.Bottom - cmd.srWindow.Top +1;
}