#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <stdbool.h>
#include <windows.h>

//========== CONSTANTES =========//
#define OBS_RAIZES 50
#define OBS_ROWS 10
#define OBS_BLOK 5

#define DIMENSAO_X 100
#define DIMENSAO_Y 50
#define OFFSET_X 2

#define SETAS     0xE0
#define KEY_UP    0x48
#define KEY_DOWN  0x50
#define KEY_LEFT  0x4B
#define KEY_RIGTH 0x4D

#define EMPTY_ROW ' '
#define BORDA_ROW 178
#define BORDA_COL 178
#define OBST_ROW  176

#define CABEC_COB '@'
#define BODY_COB  254
#define MACA_CHAR 'O'

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

    /*Para evitar processamento na 
    identificação da primeira posição*/
    ROW *cabec;
} COBRA;

typedef struct MACA{
    ROW *row;
} MACA;

typedef struct TELA{
    COBRA *cobra;
    MACA *maca;
    char **matriz;
} TELA;

//========== ASSINATURAS =========//
TELA    *cria_tela();
COBRA   *cria_cobra();
MACA    *cria_maca();
char    **cria_matriz();
DIRECAO *cria_direcao();

void imprime_cobra(COBRA *cobra);
void imprime_maca(MACA *maca);
void imprime_borda();
void imprime_obstaculo();
void imprime_mensagem(char *mensagem);

void atualiza_direcao(int tecla, DIRECAO *direcao);
void atualiza_cobra(COBRA *cobra);
void atualiza_posicao(ROW *row, DIRECAO *direcao);
void atualiza_maca();

char verifica_obstaculo(ROW *row, DIRECAO *direcao);
bool verifica_colisao(char elm);
bool verifica_maca(char elm);

void alimenta_cobra(COBRA *cobra);

ROW     *sorteia_row(DIRECAO *direcao);
ROW     **sorteia_obstaculo();
MACA    *sorteia_maca();
DIRECAO *sorteia_direcao();

void run_cursor();
void set_cursor(DWORD x,DWORD y);
char get_char_by_cursor(DWORD x,DWORD y);


//============== MAIN ============//
int main(){
    
    TELA *tela = cria_tela();
    int tecla;
    bool process;

    //Iniciando tela
    run_cursor();

    //Desenhando componentes
    imprime_borda();
    imprime_obstaculo();
    imprime_maca(tela->maca);
    imprime_cobra(tela->cobra);

    //Atualização de frames
    process = true;
    do{
        tecla = getch();

        if(tecla == SETAS){ //Código das setas
            tecla = getch(); //Direção da seta
            atualiza_direcao(tecla, tela->cobra->direcao);
        }                
    
        char elm_coli = verifica_obstaculo(
            tela->cobra->cabec, 
            tela->cobra->direcao);

        if(verifica_colisao(elm_coli)){
            imprime_mensagem("VOCE COLIDIU!");
            process = false;
            break;
        }

        if(verifica_maca(elm_coli)){
            alimenta_cobra(tela->cobra);
            atualiza_maca();
        }

        atualiza_cobra(tela->cobra);
    }while(tecla != '*' && process);

    getch();

    return 0;
}

//====== ALOCACAO DE TAD's ========//
TELA *cria_tela(){
    
    TELA *tela = (TELA*)malloc(sizeof(TELA));

    tela->cobra = cria_cobra();
    tela->maca  = cria_maca();
    tela->matriz = cria_matriz();

    return tela;
}

COBRA *cria_cobra(){
    
    int pos_x = (int)(DIMENSAO_X * OFFSET_X / 3);

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
    
    int pos_x = (int)(DIMENSAO_X * OFFSET_X / 2);

    MACA *maca = (MACA*) malloc(sizeof(MACA));
    maca->row = (ROW*) malloc(sizeof(ROW));
    maca->row->elm = MACA_CHAR;
    maca->row->prox = NULL;
    maca->row->pos_x = pos_x - pos_x % OFFSET_X; 
    maca->row->pos_y = (int)(DIMENSAO_Y / 2);

    return maca;
}

char **cria_matriz(){

    char **matriz = (char**) malloc(sizeof(char*) * DIMENSAO_Y);

    int idx_x, idx_y;
    for(idx_y = 0; idx_y < DIMENSAO_Y; idx_y++){
        matriz[idx_y] = (char*)malloc(sizeof(char) * DIMENSAO_X);
        for(idx_x = 0; idx_x < DIMENSAO_X; idx_x++){
            matriz[idx_y][idx_x] = ' ';
        }
    }

    return matriz;
}

DIRECAO *cria_direcao(){

    DIRECAO *direcao = (DIRECAO*)malloc(sizeof(DIRECAO));

    direcao->inc_x = 0;
    direcao->inc_y = 0;

    return direcao;
}

//============= DESENHOS ============//
void imprime_cobra(COBRA *cobra){

    ROW *row = cobra->final;
    
    while(row != NULL){
        set_cursor(row->pos_x,row->pos_y);
        putchar(row->elm);

        row = row->prox;
    }
}

void imprime_maca(MACA *maca){

    set_cursor(
        maca->row->pos_x,
        maca->row->pos_y
    );

    putchar(maca->row->elm);
}

void imprime_borda(){

    //Borda X
    int idx_x, idx_y, lim_x = DIMENSAO_X * OFFSET_X;
    for(idx_x = 0; idx_x < lim_x; idx_x++){
        set_cursor(idx_x,0);
        putchar(BORDA_COL);

        set_cursor(idx_x, DIMENSAO_Y -1);
        putchar(BORDA_COL);
    }

    //Borda Y   
    for(idx_y = 0; idx_y < DIMENSAO_Y; idx_y++){
        set_cursor(0,idx_y);
        putchar(BORDA_ROW);
        
        set_cursor(lim_x, idx_y);
        putchar(BORDA_ROW);
    }

}

void imprime_obstaculo(){

    ROW **obstaculos = sorteia_obstaculo();

    int idx_obs, idx_offset;
    for(idx_obs = 0; idx_obs < OBS_RAIZES; idx_obs++){
        ROW *obstaculo = obstaculos[idx_obs];

        while(obstaculo != NULL){
    
            //Iprime nos offsets
            for(idx_offset = 0; idx_offset < OFFSET_X; idx_offset++){
                set_cursor(obstaculo->pos_x + idx_offset, obstaculo->pos_y);
                putchar(obstaculo->elm);
            }

            obstaculo = obstaculo->prox;
        }
    }
}

void imprime_mensagem(char *mensagem){
    
    int idx_x, idx_y, idx_offset, size = 0;

    //Calcula tamanho da string
    while(mensagem[size] != '\0')
        size++;

    int div_y = DIMENSAO_Y / 2, s_x = (DIMENSAO_X * OFFSET_X / 2) - (size / 2);

    //Borda X
    for(idx_x = -2; idx_x < size +2; idx_x++){

            //Iprime nos offsets
            for(idx_offset = 0; idx_offset < OFFSET_X; idx_offset++){
                int pos_x = s_x + idx_x + idx_offset;

                //Borda externa
                set_cursor(pos_x, div_y -2);
                putchar(178);

                set_cursor(pos_x, div_y +2);
                putchar(178);

                //Borda interna
                set_cursor(pos_x, div_y -1);
                putchar(EMPTY_ROW);

                set_cursor(pos_x, div_y);
                putchar(EMPTY_ROW);

                set_cursor(pos_x, div_y +1);
                putchar(EMPTY_ROW);
            }
    }



    //Mensagem    
    set_cursor(s_x, div_y);
    puts(mensagem);
}

//=========== ATUALIZAÇÕES ==========//
void atualiza_cobra(COBRA *cobra){

    ROW *fim = cobra->final;
    ROW *cab = cobra->cabec;

    //Define corpo onde está a cabeça
    set_cursor(cab->pos_x, cab->pos_y);
    putchar(BODY_COB);

    //Limpa ultima posicao
    set_cursor(fim->pos_x, fim->pos_y);
    putchar(EMPTY_ROW);

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

    set_cursor(cab->pos_x, cab->pos_y);
    putchar(cab->elm);
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

void atualiza_maca(){    
    imprime_maca(sorteia_maca());
}

//============ STATUS ===========//
void alimenta_cobra(COBRA *cobra){
    
    ROW *fim = cobra->final;
    ROW *novo = (ROW*) malloc(sizeof(ROW));

    novo->pos_x = fim->pos_x;
    novo->pos_y = fim->pos_y;
    novo->elm = BODY_COB;
    novo->prox = fim;

    cobra->final = novo;
}

//======== VALIDAÇÕES =========//
char verifica_obstaculo(ROW *row, DIRECAO *direcao){
        
    int new_pos_x = row->pos_x + direcao->inc_x;
    int new_pos_y = row->pos_y + direcao->inc_y;

    return get_char_by_cursor(new_pos_x, new_pos_y);
}

bool verifica_colisao(char elm){
    return (elm != EMPTY_ROW && elm != MACA_CHAR);
}

bool verifica_maca(char elm){
    return (elm == MACA_CHAR);
}

//========== SORTEIOS ==========//
MACA *sorteia_maca(){

    MACA *maca = cria_maca();
    DIRECAO *direcao = cria_direcao();
    srand(time(NULL));
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
        row->pos_x = (rand() % (DIMENSAO_X -1) +1) * OFFSET_X;
        row->pos_y = (rand() % (DIMENSAO_Y -1) +1);
    }while(verifica_obstaculo(row, direcao) != EMPTY_ROW);

    return row;
}

ROW **sorteia_obstaculo(){
    
    ROW **obstaculos = (ROW**) malloc(sizeof(ROW*) * OBS_RAIZES);
    int idx_raiz = 0, idx_row = 0;
    DIRECAO *direcao = sorteia_direcao();
    srand(time(NULL));

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
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD pos = {0,0};
    DWORD esc;
    DWORD tam = csbi.dwSize.X *csbi.dwSize.Y;
    FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE),EMPTY_ROW,tam,pos,&esc);
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),pos);
}

void set_cursor(DWORD x,DWORD y){
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),coord);
}

char get_char_by_cursor(DWORD x,DWORD y){
    char buf[2]; 
    COORD coord = {x,y}; 
    DWORD num_read;
	HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE);
	ReadConsoleOutputCharacter(hStd,(LPTSTR)buf,1,coord,(LPDWORD)&num_read);
	return buf[0];
}