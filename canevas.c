// canevas.c   pour projet Genealogie SDA2 2024-25�
//

// NOM: Veverita PRENOM: Ion
///////////////////////////////////

#include "base.h"
#define LG_MAX 64
#define omega 0

// Structures
typedef struct s_date { Nat jour, mois, annee; } Date;
typedef Nat Ident;

typedef struct s_individu {
    Car nom[LG_MAX];
    Date naiss;
    Date deces;
    Ident id;
    Nat ipere, imere, icadet, ifaine;
} *Individu;

typedef struct s_genealogie {
    Individu* tab;
    Nat nb_individus, id_cur, taille_max_tab, taille_max_rang;
    Nat* rang;
} *Genealogie;

// PARTIE 1: PROTOTYPES des operations imposees
Ent compDate(Date d1, Date d2);
void genealogieInit(Genealogie* g);
void genealogieFree(Genealogie* g);
Individu nouvIndividu(Ident i, Chaine s, Ident p, Ident m, Date n, Date d);
void freeIndividu(Individu id);
// selecteurs
Chaine nomIndividu(Individu ind);
Date naissIndividu(Individu ind);
Nat cardinal(Genealogie g);
Individu kieme(Genealogie g, Nat k);
Individu getByIdent(Genealogie g, Ident i);
Nat getPos(Genealogie g, Chaine name);
Individu getByName(Genealogie g, Chaine name, Date naissance);
// modificateurs
Ident adj(Genealogie g, Chaine s, Ident p, Ident m, Date n, Date d);
void insert(Genealogie g, Nat pos, Chaine s, Ident p, Ident m, Date n, Date d);
void adjFils(Genealogie g, Ident idx, Ident fils, Ident pp, Ident mm);

//PARTIE 2: PROTOTYPES des operations imposees
void affiche_freres_soeurs(Genealogie g, Ident x, Chaine buff);
void affiche_enfants(Genealogie g, Ident x, Chaine buff);
void affiche_cousins(Genealogie g, Ident x, Chaine buff);
void affiche_oncles(Genealogie g, Ident x, Chaine buff);

//PARTIE 3: PROTOTYPES des operations imposees
void deviennent_freres_soeurs(Genealogie g, Ident x, Ident y);
void devient_pere(Genealogie g, Ident x, Ident y);
void devient_mere(Genealogie g, Ident x, Ident y);

//PARTIE 4: PROTOTYPES des operations imposees
Bool estAncetre(Genealogie g, Ident x, Ident y);
Bool ontAncetreCommun(Genealogie g, Ident x, Ident y);
Ident plus_ancien(Genealogie g, Ident x);
void affiche_parente(Genealogie g, Ident x, Chaine buf);
void affiche_descendance(Genealogie g, Ident x, Chaine buf);

// PROTOTYPES DE VOS FONCTIONS INTERMEDIAIRES
void affiche_descendance_recursive(Genealogie g, Ident x, int level, Chaine* levels, int* maxLevel);
Chaine chercherChaine(Chaine haystack, Chaine needle);
void addToBuffer(Chaine buff, Chaine nom);
Nat dateNull(Date d1);
void addToLevelBuffer(Chaine buffer, Chaine name);
Bool nameExistsInBuffer(Chaine buffer, Chaine name);
void affiche_parente_recursive(Genealogie g, Ident x, Nat level, Chaine* levels, Nat* maxLevel);




// PARTIE 1:----------------------------------------------------------------
// < 0 Si d1 est plus ancien, = 0 si egal et > 0 si d2 est plus ancien
Ent compDate(Date d1, Date d2) {
    if (d1.annee != d2.annee) {return d1.annee - d2.annee;}
    if (d1.mois != d2.mois) {return d1.mois - d2.mois;}
    return d1.jour - d2.jour;}

// Initialisation du chaque champ d'un genealogie
void genealogieInit(Genealogie *g) {
    *g = MALLOC(struct s_genealogie);
    if (*g == NULL) {return;}
    (*g)->nb_individus = 0;
    (*g)->id_cur = 0;
    (*g)->taille_max_tab = 10; 
    (*g)->tab = MALLOCN(Individu, (*g)->taille_max_tab);
    if ((*g)->tab == NULL) {return;}
    (*g)->taille_max_rang = 10;
    (*g)->rang = MALLOCN(Nat, (*g)->taille_max_rang);
    if ((*g)->rang == NULL) {return;}}

// Liberation de la memoire d'un genealogie
void genealogieFree(Genealogie *g) {
    if (*g == NULL) {return;}

    for (Nat i = 0; i < (*g)->nb_individus; i++) {
        FREE((*g)->tab[i]);
    }
    FREE((*g)->tab);
    FREE((*g)->rang);
    FREE(*g);
    *g = NULL;} // Au cas ou

// Initialisation d'un individu dans la memoire
Individu nouvIndividu(Ident i, Chaine s, Ident p, Ident m, Date n, Date d) {
    Individu nouvInd = MALLOC(struct s_individu);
    if (nouvInd == NULL) {return NULL;}
    nouvInd->id = i;
    chaineCopie(nouvInd->nom, s);
    nouvInd->naiss = n;
    nouvInd->deces = d;
    nouvInd->ipere = p;
    nouvInd->imere = m;
    nouvInd->icadet = 0;
    nouvInd->ifaine = 0;
    return nouvInd;}

// Liberation de la memoire d'un individu
void freeIndividu(Individu ind) {
    if (ind == 0) {return;}
    FREE(ind);}

// selecteurs-----------------------------------------------------------------------------------------------

// Nom d'individu
Chaine nomIndividu(Individu ind) {
    return ind->nom;}

// Date de naissance d'un individu
Date naissIndividu(Individu ind) {
    return ind->naiss;}

// Nombre des individus d'un genealogie
Nat cardinal(Genealogie g) {
    return g->nb_individus;}

// K-ieme individu du tableau du genealogie
Individu kieme(Genealogie g, Nat k) {
    if (g == NULL || k >= g->nb_individus) {return NULL;}
    return g->tab[k];}

// Renvoie un Individu trouve par son id
Individu getByIdent(Genealogie g, Ident i) {
    if (g == NULL || i == 0 || i > g->id_cur) {return NULL;}
    return kieme(g, (g->rang[i - 1]));}

// Optenir la position d'insertion par nom
Nat getPos(Genealogie g, Chaine name) {
    if (g == NULL || cardinal(g) == 0) return 0;
    Nat left = 0; Nat right = g->nb_individus - 1; Nat result = g->nb_individus; 
    while (left <= right) {
        Nat mid = left + (right - left) / 2; 
        if (mid >= g->nb_individus || g->tab[mid] == NULL) {
            return result;}
        int cmp = chaineCompare(name, g->tab[mid]->nom);
        if (cmp <= 0) {result = mid; right = mid - 1;
        } else {left = mid + 1;}}
    return result;
}

// Renvoie un Individu trouve par son nom et date de naissance
Individu getByName(Genealogie g, Chaine name, Date naissance) {
    Nat pos = getPos(g, name);
    Individu best = NULL;
    while (pos < g->taille_max_tab && g->tab[pos] != NULL &&
           chaineCompare(g->tab[pos]->nom, name) == 0) {
        if (dateNull(naissance) || compDate(g->tab[pos]->naiss, naissance) == 0) {
            if (best == NULL || compDate(g->tab[pos]->naiss, best->naiss) > 0) {best = g->tab[pos];}}
        pos++;}
    return best;}

// modificateurs-----------------------------------------------------------------------------------------------
//Insertion d'un individu en position pos avec tous les champs p m n d deja precises
void insert(Genealogie g, Nat pos, Chaine s, Ident p, Ident m, Date n, Date d) {
    Individu nouvInd = nouvIndividu(++(g->id_cur), s, p, m, n, d); if (nouvInd == NULL) {g->id_cur--; return; } //Creation d'individu
    if (pos > g->nb_individus) return;
    Nat capacite_indv = cardinal(g);
    if (g->nb_individus == g->taille_max_rang) { // Reallocation du rang si besoin (x2)
        Nat* new_rang = REALLOC(g->rang, Nat, g->taille_max_rang * 2);
        if (new_rang == NULL) return;
        g->rang = new_rang;
        g->taille_max_rang *= 2;}
    if (g->nb_individus == g->taille_max_tab) { // Reallocation du tab si besoin (x2)
        Individu* new_tab = REALLOC(g->tab, Individu, g->taille_max_tab * 2);
        if (new_tab == NULL) return;
        g->tab = new_tab;
        g->taille_max_tab *= 2;}
    while (capacite_indv > pos) { // Parcours du dernier element jusqu'a pos pour decaler les Individus existants
        g->rang[g->tab[capacite_indv - 1]->id - 1] = capacite_indv;
        g->tab[capacite_indv] = g->tab[capacite_indv - 1]; 
        capacite_indv--;}
    // Insertion dans pos
    g->tab[pos] = nouvInd;
    g->nb_individus += 1;
    g->rang[g->id_cur - 1] = pos;
}



//Insertion dans la liste des fils de idx, avec idx deja insere dans la genealogie (Mettons a jour les liens parents-fils)
void adjFils(Genealogie g, Ident idx, Ident fils, Ident p, Ident m) {
    if (g == NULL || idx == omega || (p == omega && m == omega)) return;
    Individu newChild = getByIdent(g, idx);
    if (newChild == NULL) return;
    newChild->ipere = p;
    newChild->imere = m;
    newChild->icadet = omega;
    newChild->ifaine = omega;
    Individu pere = (p != omega) ? getByIdent(g, p) : NULL;
    Individu mere = (m != omega) ? getByIdent(g, m) : NULL;
    // Si les parents on pas des enfants
    if (fils == omega) {
        if (pere != NULL) pere->ifaine = idx;
        if (mere != NULL) mere->ifaine = idx;
        return;}
    Individu filsAine = getByIdent(g, fils); if (filsAine == NULL) return;
    int dateComp = compDate(newChild->naiss, filsAine->naiss); // Comparons les date de naissance avec l'enfaine aine
    if (dateComp < 0) { // Si idx est le plus grand
        newChild->icadet = fils;
        if (pere != NULL && pere->ifaine == fils) pere->ifaine = idx;
        if (mere != NULL && mere->ifaine == fils) mere->ifaine = idx;
    } else if (dateComp == 0) { // Si ils ont la meme date de naissance alors on tri par nom
        int nomComp = chaineCompare(newChild->nom, filsAine->nom);
        if (nomComp < 0) { 
            newChild->icadet = fils;
            if (pere != NULL && pere->ifaine == fils) pere->ifaine = idx;
            if (mere != NULL && mere->ifaine == fils) mere->ifaine = idx;
        } else {
            newChild->icadet = filsAine->icadet;
            filsAine->icadet = idx;}
    } else { // Si l'enfant aine reste en tete, on cherche avec une boucle
        Individu autreFils = getByIdent(g, filsAine->icadet); if (autreFils == NULL) {filsAine->icadet = idx; return;}
        while(autreFils != NULL && compDate(newChild->naiss, autreFils->naiss) > 0) {
            filsAine = autreFils;
            autreFils = getByIdent(g, autreFils->icadet);}
        if (autreFils == NULL) {filsAine->icadet = idx; return;}
        filsAine->icadet = idx; newChild->icadet = autreFils->id;}}


//Insertion d'un individu dans la genealogie par default
Ident adj(Genealogie g, Chaine s, Ident p, Ident m, Date n, Date d) {
    if (s == NULL || s[0] == 0 || n.jour == 0 || n.mois == 0 || n.annee == 0) return omega;
    Nat pos = getPos(g, s);
    // Verifions is la position est valide et qu'il n'y a pas l'individu deja en genealogie
    if (pos < g->nb_individus && g->tab[pos] != NULL) {
        Individu existing = g->tab[pos];
        if (chaineCompare(s, existing->nom) == 0 && 
            compDate(n, existing->naiss) == 0) {
            return omega; }}
    insert(g, pos, s, p, m, n, d);
    
    // On mets a jour les liaison parentales
    Ident new_id = g->id_cur; Ident faine;
    if (p != omega) {faine = getByIdent(g, p)->ifaine;
    } else if (m != omega) {faine = getByIdent(g, m)->ifaine;
    } else {faine = omega;}    
    adjFils(g, new_id, faine, p, m);
    return new_id;}

//PARTIE 2:-----------------------------------------------------------------------------------------------
void affiche_freres_soeurs(Genealogie g, Ident x, Chaine buff) {
    buff[0] = '\0';
    if (g == NULL || x == omega) return;
    Individu current = getByIdent(g, x);
    if (current == NULL) return;
    Ident idPere = current->ipere;
    Ident idMere = current->imere;
    if (idPere == omega && idMere == omega) return; // Pour avoir des freres ou soeurs il faut qu'au moins un parent existe

    Ident parentId = (idPere != omega) ? idPere : idMere; // On prend l'aine des parents
    Individu parent = getByIdent(g, parentId);
    if (parent == NULL || parent->ifaine == omega) return;
    Ident enf = parent->ifaine;
    Individu enfant;
    while (enf != omega) { // On parcours tous les enfants du parents de x
        enfant = getByIdent(g, enf);
        if (enfant == NULL) return;
        if (chaineCompare(enfant->nom, current->nom) != 0) {addToBuffer(buff, enfant->nom);} // nom de x exclu 
        enf = enfant->icadet;}}

void affiche_enfants(Genealogie g, Ident x, Chaine buff) {
    buff[0] = '\0';
    if (g == NULL || x == omega) return;
    Individu parent = getByIdent(g, x);
    if (parent == NULL || parent->ifaine == omega) return;
    Ident enf = parent->ifaine; // On recupere le premier enfant
    Individu enfant;
    while (enf != omega) { // Parcourons dans une boucle
        enfant = getByIdent(g, enf);
        if (enfant == NULL) return;
        addToBuffer(buff, enfant->nom);
        enf = enfant->icadet; }}

void affiche_cousins(Genealogie g, Ident x, Chaine buff) {
    buff[0] = '\0';
    if (g == NULL || x == omega) return;
    Individu current = getByIdent(g, x);
    if (current == NULL) return;
    Ident parents[2] = {current->ipere, current->imere};
    
    for (int i = 0; i < 2; i++) { // Parcours des freres/soeurs de chaque parent
        if (parents[i] != omega) {
            Individu parent = getByIdent(g, parents[i]);        
            Ident grandparents[2] = {parent->ipere, parent->imere};
            for (int j = 0; j < 2; j++) { 
                if (grandparents[j] != omega) {
                    Individu grandparent = getByIdent(g, grandparents[j]);
                    Ident siblingId = grandparent->ifaine; // On recupere la liste des freres/soeurs de chaque parent
                    while (siblingId != omega) {
                        Individu oncle = getByIdent(g, siblingId);
                        if (oncle && oncle->id != parent->id) {Ident cousinId = oncle->ifaine;
                            while (cousinId != omega) {
                                Individu cousin = getByIdent(g, cousinId); // On reviens vers le cousin
                                if (cousin && chercherChaine(buff, cousin->nom) == NULL) { // Si c'est pas un parent et pas deja dans le buff alors on ajoute
                                addToBuffer(buff, cousin->nom);} 
                                cousinId = cousin->icadet;}}
                            siblingId = oncle->icadet;}}}}}}

void affiche_oncles(Genealogie g, Ident x, Chaine buff) {
    buff[0] = '\0';
    if (g == NULL || x == omega) return;
    Individu current = getByIdent(g, x);
    if (current == NULL) return;
    Ident parents[2] = {current->ipere, current->imere}; 
    for (int i = 0; i < 2; i++) { // Parcours des freres/soeurs de chaque parent
        if (parents[i] != omega) {
            Individu parent = getByIdent(g, parents[i]);
            Ident grandparents[2] = {parent->ipere, parent->imere}; 
            for (int j = 0; j < 2; j++) {
                if (grandparents[j] != omega) {
                    Individu grandparent = getByIdent(g, grandparents[j]);
                    Ident siblingId = grandparent->ifaine; // On recupere la liste des freres/soeurs de chaque parent
                    while (siblingId != omega) {
                        Individu oncle = getByIdent(g, siblingId);                
                        if (oncle->id != current->imere && oncle->id != current->ipere) {
                            if (chercherChaine(buff, oncle->nom) == NULL) { // Si pas deja dans le buff
                                addToBuffer(buff, oncle->nom);}} // On ajout un oncle
                            siblingId = oncle->icadet;}}}}}}


//PARTIE 3:-----------------------------------------------------------------------------------------------
// Pre: Les deux ont les memes parents || un des deux a les parents omega 
void deviennent_freres_soeurs(Genealogie g, Ident x, Ident y) {
    if (g == NULL || x == omega || y == omega || x == y) return;
    Individu indX = getByIdent(g, x);
    Individu indY = getByIdent(g, y);

    if (indX == NULL || indY == NULL) return;
    if (indX->ipere != omega && indY->ipere != omega && indX->ipere != indY->ipere) return;
    if (indX->imere != omega && indY->imere != omega && indX->imere != indY->imere) return;
    // Si les deux ont des parents differents, ils ne peuvent pas devenir freres ou soeurs

    Individu sourceInd = NULL;
    Individu targetInd = NULL;
    
    if ((indX->ipere != omega || indX->imere != omega) && 
        (indY->ipere == omega && indY->imere == omega)) {
        // X has parent(s), Y has none
        sourceInd = indX;
        targetInd = indY;
    } else if ((indY->ipere != omega || indY->imere != omega) && 
               (indX->ipere == omega && indX->imere == omega)) {
        // Y has parent(s), X has none
        sourceInd = indY;
        targetInd = indX;
    } else {return;}

    Ident sourceParent = (sourceInd->imere != omega) ? sourceInd->imere : sourceInd->ipere;
    Ident filsAine = (getByIdent(g, sourceParent) != NULL) ? getByIdent(g, sourceParent)->ifaine : omega;
    Ident sourceP = sourceInd->ipere;
    Ident sourceM = sourceInd->imere;
    adjFils(g, targetInd->id, filsAine, sourceP, sourceM);}

void devient_pere(Genealogie g, Ident x, Ident y) {
    if (g == NULL || x == omega || y == omega || x == y) return;
    Individu pere = getByIdent(g, x);
    Individu enfant = getByIdent(g, y);
    Bool forLoop = true;
    if (pere == NULL || enfant == NULL) return;
    if (enfant->ipere != omega && enfant->ipere != x) return;
    enfant->ipere = x;
    if (pere->ifaine == omega) {pere->ifaine = y;
    } else {
        Ident currentChildId = pere->ifaine;
        Individu currentChild;
        while (currentChildId != omega && forLoop) {
            currentChild = getByIdent(g, currentChildId);            
            if (currentChildId == y) return;
            if (currentChild->icadet == omega) {
                currentChild->icadet = y; forLoop =false;}
            currentChildId = currentChild->icadet;}}}

void devient_mere(Genealogie g, Ident x, Ident y) {
    if (g == NULL || x == omega || y == omega || x == y) return;
    Individu mere = getByIdent(g, x);
    Individu enfant = getByIdent(g, y);
    Bool forLoop = true;
    if (mere == NULL || enfant == NULL) return;
    if (enfant->imere != omega && enfant->imere != x) return;
    enfant->imere = x;
    if (enfant->ipere != omega) {return;}
    if (mere->ifaine == omega) {mere->ifaine = y;
    } else {
        Ident currentChildId = mere->ifaine;
        Individu currentChild;
        while (currentChildId != omega && forLoop) {
            currentChild = getByIdent(g, currentChildId);
            if (currentChildId == y) return;
            if (currentChild->icadet == omega) {
                currentChild->icadet = y; forLoop = false;}
            currentChildId = currentChild->icadet;}}}

//PARTIE 4:-----------------------------------------------------------------------------------------------
//PRE: None
Bool estAncetre(Genealogie g, Ident x, Ident y) {
    if (x == omega || y == omega) return false;
    Individu ind = getByIdent(g,y);
    if (ind->ipere == x || ind->imere == x) return true;
    if (ind->ipere == omega && ind->imere == omega) return false;
    else return estAncetre(g,x,ind->ipere) || estAncetre(g,x,ind->imere);} 

//PRE: None
Bool ontAncetreCommun(Genealogie g, Ident x, Ident y) {
    if (x == omega || y == omega) return false;
    Individu ind_x = getByIdent(g,x);
    Individu ind_y = getByIdent(g,y);
    if (ind_x == NULL || ind_y == NULL) return false;
    if (ind_x->ipere == ind_y->ipere && ind_x->ipere != omega) return true;
    if (ind_x->imere == ind_y->imere && ind_x->imere != omega) return true;
    else return ontAncetreCommun(g,ind_x->ipere,ind_y->ipere) || ontAncetreCommun(g,ind_x->ipere,ind_y->imere) || ontAncetreCommun(g,ind_x->imere,ind_y->ipere) || ontAncetreCommun(g,ind_x->imere,ind_y->imere);}

//PRE: None
Ident plus_ancien(Genealogie g, Ident x){
    if (x == omega) return omega;
    Individu ind = getByIdent(g,x);
    if (ind == NULL) return omega;
    if (ind->ipere == omega && ind->imere == omega) return x;
    if (ind->ipere == omega) return plus_ancien(g,ind->imere);
    if (ind->imere == omega) return plus_ancien(g,ind->ipere);
    else return compDate(getByIdent(g,plus_ancien(g,ind->ipere))->naiss, getByIdent(g,plus_ancien(g,ind->imere))->naiss) < 0 ? plus_ancien(g,ind->ipere) : plus_ancien(g,ind->imere);}

void affiche_parente(Genealogie g, Ident x, Chaine buff) {
    buff[0] = '\0'; 
    if (g == NULL || x == omega) return;
    Individu person = NULL;
    Bool forLoop = true;
    for (Nat i = 0; i < g->nb_individus && forLoop; i++) {
        if (g->tab[i]->id == x) {
            person = g->tab[i]; forLoop = false;}}
    Chaine levels[11]; 
    for (Nat i = 0; i <= 10; i++) {
        levels[i] = MALLOCN(Car, 1000); 
        levels[i][0] = '\0';}
    Nat maxLevel = 0;
    if (person == NULL) {
        for (Nat i = 0; i <= 10; i++) {
            FREE(levels[i]);}
        return;}
    if (person->ipere != omega) {affiche_parente_recursive(g, person->ipere, 1, levels, &maxLevel);}
    if (person->imere != omega) {affiche_parente_recursive(g, person->imere, 1, levels, &maxLevel);}
    for (Nat i = 1; i <= maxLevel; i++) {
        if (levels[i][0] != '\0') {
            chaineConcat(buff, " - ");
            char levelStr[3];
            if (i < 10) { levelStr[0] = '0' + i; levelStr[1] = '\0';
            } else {levelStr[0] = '1';levelStr[1] = '0';levelStr[2] = '\0';}
            chaineConcat(buff, levelStr);
            chaineConcat(buff, " :\n");
            chaineConcat(buff, levels[i]);
            chaineConcat(buff, "\n");}}
    for (Nat i = 0; i <= 10; i++) {
        FREE(levels[i]);}}

void affiche_descendance(Genealogie g, Ident x, Chaine buff) {
    buff[0] = '\0';  
    if (g == NULL || x == omega) return;
    Chaine levels[10];
    for (int i = 0; i < 10; i++) {
        levels[i] = MALLOCN(Car, 500);
        levels[i][0] = '\0';}
    int maxLevel = 0;  
    affiche_descendance_recursive(g, x, 1, levels, &maxLevel);
    int buf_pos = 0;
    for (int i = 1; i <= maxLevel; i++) {
        buff[buf_pos++] = '-';
        buff[buf_pos++] = ' ';
        if (i >= 10) {buff[buf_pos++] = '1'; buff[buf_pos++] = '0';
        } else {buff[buf_pos++] = '0' + i;}
        buff[buf_pos++] = ' ';
        buff[buf_pos++] = ':';
        buff[buf_pos++] = '\n';
        for (int j = 0; levels[i][j] != '\0'; j++) {
            buff[buf_pos++] = levels[i][j];}
        buff[buf_pos++] = '\n';}
    buff[buf_pos] = '\0';  
    for (int i = 0; i < 10; i++) {
        FREE(levels[i]);}}

// 
/// VOS FONCTIONS AUXILIAIRES 
/// ///////////////////////////////////////////////////////
/// 
// Date 0/0/0
Nat dateNull(Date d1) {
    return d1.annee == 0 && d1.mois == 0 && d1.jour == 0;}

void addToBuffer(Chaine buff, Chaine nom) {
    if (buff[0] == '\0' || (buff[0] >= 0 && buff[0] <= 32)) {
        int j = 0;
        while (nom[j] != '\0') {
            buff[j] = nom[j];
            j++;}
        buff[j] = '\0';return;
    } else {
        int i = 0;
        while (buff[i] != '\0') {i++;}
        buff[i++] = ' ';
        int j = 0;
        while (nom[j] != '\0') {
        buff[i++] = nom[j++];
        buff[i] = '\0';}}}

Chaine chercherChaine(Chaine haystack, Chaine needle) {
    if (!*needle) return haystack;
    for (Chaine p = haystack; *p; p++) {
        Chaine h = p;
        Chaine n = needle;
        while (*h && *n && *h == *n) {h++;n++;}
        if (!*n) return p; }
    return NULL; 
}


void affiche_descendance_recursive(Genealogie g, Ident x, int level, Chaine* levels, int* maxLevel) {
    if (g == NULL || x == omega) return;
    Individu current = getByIdent(g, x);
    if (current == NULL || current->ifaine == omega) return;  
    if (level > *maxLevel) {
        *maxLevel = level;}
    Ident childId = current->ifaine;
    while (childId != omega) {
        Individu child = getByIdent(g, childId);
        if (chercherChaine(levels[level], child->nom) == NULL) {
            addToBuffer(levels[level], child->nom);}
        affiche_descendance_recursive(g, childId, level + 1, levels, maxLevel);
        childId = child->icadet;}}

void addToLevelBuffer(Chaine buffer, Chaine name) {
    if (buffer[0] != '\0') {chaineConcat(buffer, " ");}
    chaineConcat(buffer, name);}

Bool nameExistsInBuffer(Chaine buffer, Chaine name) {
    Chaine found = strstr(buffer, name);
    if (found == NULL) return false;
    Nat nameLen = chaineLongueur(name);
    Bool validBefore = (found == buffer) || (*(found-1) == ' ');
    Bool validAfter = (found[nameLen] == '\0') || (found[nameLen] == ' ');
    return validBefore && validAfter;}

void affiche_parente_recursive(Genealogie g, Ident x, Nat level, Chaine* levels, Nat* maxLevel) {
    if (g == NULL || x == omega) return;
    Individu current = NULL;
    Bool forLoop = true;
    for (Nat i = 0; i < g->nb_individus && forLoop; i++) {
        if (g->tab[i]->id == x) {
            current = g->tab[i]; forLoop = false;}}
    if (current == NULL) return;
    if (level > *maxLevel) {
        *maxLevel = level;}
    if (!nameExistsInBuffer(levels[level], current->nom)) {
        addToLevelBuffer(levels[level], current->nom);}
    for (Nat i = 0; i < g->nb_individus; i++) {
        Individu person = g->tab[i];
        if (person->id != x && 
            ((person->ipere == current->ipere && person->ipere != omega) || 
             (person->imere == current->imere && person->imere != omega))) {
            if (!nameExistsInBuffer(levels[level], person->nom)) {
                addToLevelBuffer(levels[level], person->nom);}}}
    if (current->ipere != omega) {affiche_parente_recursive(g, current->ipere, level + 1, levels, maxLevel);}
    
    if (current->imere != omega) {affiche_parente_recursive(g, current->imere, level + 1, levels, maxLevel);}}

// 
/// MAIN
/// ///////////////////////////////////////////////////////
/// 
int main()
{
    Car buf[500];
    Genealogie g;

    printf("******* Init:\n\n");
    genealogieInit(&g);
    printf("done.\n");

    printf("\n******* adj+getByIndent:\n");
    Date dnull = { 0,0,0 };
// Famille Potter
    Date jhen = { 16,2,1867 }; Date jhed = { 21,11,1932 };
    Ident ihep = adj(g, "Henri", 0, 0, jhen, jhed);

    Date jfn = { 30,7,1905 }; Date jfd = { 2,3,1979 };
    Ident ijfl = adj(g, "Fleamont", ihep, 0, jfn, jfd);

    Date jeu = { 12,6,1907 }; Date jed = { 14, 1,1979 };
    Ident ijm = adj(g, "Euphemia", 0, 0, jeu, jed);

    Date jpn = { 27, 3, 1960 }; Date jpd = { 29, 7, 1981 };
    Ident ijp = adj(g, "James", ijfl, ijm, jpn, jpd);

    Date lpn = { 30, 1, 1960 }; Date lpd = { 29, 7, 1981 };
    Ident ilp = adj(g, "Lily", 0, 0, lpn, lpd);

    Date hn = { 31, 7, 1980 };
    Ident ih = adj(g, "Harry", ijp, ilp, hn, dnull);

    // Famille Weasley
    Date an = { 6, 2, 1950 };
    Ident iaw = adj(g, "Arthur", 0, 0, an, dnull);

    Date dpre = { 8,4, 1910 }; Date ddpre = { 23, 10, 1968 };
    Ident ipre = adj(g, "Prewett", 0, 0, dpre, ddpre);

    Date dfab = { 12, 5, 1946 }; Date ddfab = { 21,12, 1982 };
    Ident ifab = adj(g, "Fabian", ipre, 0, dfab, ddfab);

    Date mn = { 30, 10, 1949 };
    Ident imw = adj(g, "Molly", ipre, 0, mn, dnull);

    // ajouter ici les autres individus
    Date dginny = {11, 8, 1981};
    Ident ig = adj(g, "Ginny", iaw, imw, dginny, dnull);

    Date dron = {1, 3, 1980};
    Ident ir = adj(g, "Ron", iaw, imw, dron, dnull);

    Date dalbus = {15, 8, 2006};  // estimated date
    Ident ia2 = adj(g, "Albus", ih, ig, dalbus, dnull);

    Date dbill = {29, 11, 1970};
    Ident ibill = adj(g, "Bill", iaw, imw, dbill, dnull);

    Date djames2 = {12, 8, 2004};  // estimated date
    Ident ij = adj(g, "James", ih, ig, djames2, dnull);

    Date dhermione = {19, 9, 1979};
    Ident ihg = adj(g, "Hermione", 0, 0, dhermione, dnull);

    // Fred Weasley
    Date dfred = {1, 4, 1978};
    Ident ifred = adj(g, "Fred", iaw, imw, dfred, dnull);

    // George Weasley
    Date dgeorge = {1, 4, 1978};
    Ident igeorge = adj(g, "George", iaw, imw, dgeorge, dnull);

    Date drose = {15, 2, 2006};  // estimated date
    Ident irose = adj(g, "Rose", ir, ihg, drose, dnull);

    // Lily Luna Potter (daughter of Harry and Ginny)
    Date dlily2 = {17, 2, 2008};  // estimated date
    Ident ilily2 = adj(g, "Lily", ih, ig, dlily2, dnull);


  for (Nat i = 0; i < cardinal(g); i++) {
        printf("%s\n", nomIndividu(kieme(g,i)));
    }
    printf("nb_individus: %d\n", cardinal(g));
    printf("Identifiant de Fabian: %u (must be 9)\n", ifab);
    printf("Identifiant de Arthur: %u (must be 7)\n", iaw);

    printf("\nAdding more people:\n");
    Date dgid = { 7, 2, 1945 }; Date ddgid = { 21,12, 1982 };
    Ident igid = adj(g, "Gideon", 0, 0, dgid, ddgid);
    printf("Linking Gideon as brother of Molly\n");
    deviennent_freres_soeurs(g, igid, imw);
    Date dhugo = { 19,4,2008 };
    Ident ihugo = adj(g, "Hugo", 0, 0, dhugo, dnull);
    printf("Linking Hugo as son of Hermione\n");
    devient_mere(g, ihg, ihugo);

    printf("\nTry to add a double Harry:\n");
    Date hu2n = { 31, 7, 1980 };
    Ident ihu2 = adj(g, "Harry", 0, 0, hu2n, dnull);
    if (ihu2 != omega) printf("oh, no! Added double Harry!\n"); else printf("No double Harry: OK!\n");
    printf("Adding more Albus...\n");
    Date ho2n = { 1,8, 1909 };
    Ident itho2 = adj(g, "Albus", 0, 0, ho2n, dnull);
    if (itho2 == omega) printf("what! Could not add 2nd Albus!\n"); else printf("2nd Albus added: OK!\n");
    Date ho3n = { 2,2, 1832 };
    Ident itho3 = adj(g, "Albus", 0, 0, ho3n, dnull);
    if (itho3 == omega) printf("what! Could not add 3rd Albus!\n"); else printf("3rd Albus added: OK!\n");
    Individu idch = getByName(g, "Albus", dnull);
    if (idch != NULL) printf("Youngest Albus, date birth %d:%d:%d\n", naissIndividu(idch).jour, naissIndividu(idch).mois, naissIndividu(idch).annee);
    else printf("what? no Albus! There is a serious problem here...\n");
    printf("Now nb_individus: %d\n", cardinal(g));


    printf("\n******* fratrie:\n");
    printf("Freres/Soeurs de %s:\n", nomIndividu(getByIdent(g, ig)));
    buf[0] = 0;  affiche_freres_soeurs(g, ig, buf);
    printf("%s\n", buf);
    printf("Freres/Soeurs de %s:\n", nomIndividu(getByIdent(g, imw)));
    buf[0] = 0;  affiche_freres_soeurs(g, imw, buf);
    printf("%s\n", buf);

    printf("\n******* les enfants:\n");
    printf("Enfants de %s:\n", nomIndividu(getByIdent(g, ipre)));
    buf[0] = 0;  affiche_enfants(g, ipre, buf);
    printf("%s\n", buf);
    printf("Enfants de %s:\n", nomIndividu(getByIdent(g, imw)));
    buf[0] = 0;  affiche_enfants(g, imw, buf);
    printf("%s\n", buf);
    printf("Enfants de %s:\n", nomIndividu(getByIdent(g, ir)));
    buf[0] = 0;  affiche_enfants(g, ir, buf);
    printf("%s\n", buf);

    printf("\n******* les cousins:\n");
    printf("Cousins de %s:\n", nomIndividu(getByIdent(g, irose)));
    buf[0] = 0;   affiche_cousins(g, irose, buf);
    printf("%s\n", buf);

    printf("\n******* les oncles et tantes:\n");
    printf("Oncles/tantes de %s:\n", nomIndividu(getByIdent(g, ij)));
    buf[0] = 0;  affiche_oncles(g, ij, buf);
    printf("%s\n", buf);
    printf("Oncles/tantes de %s:\n", nomIndividu(getByIdent(g, ir)));
    buf[0] = 0;  affiche_oncles(g, ir, buf);
    printf("%s\n", buf);


    printf("\n******* les ancetres:\n");
    printf("%s ancetre de %s: %s\n", nomIndividu(getByIdent(g, ijfl)), nomIndividu(getByIdent(g, ia2)), estAncetre(g, ijfl, ia2) ? "oui" : "non");
    printf("%s ancetre de %s: %s\n", nomIndividu(getByIdent(g, ijfl)), nomIndividu(getByIdent(g, irose)), estAncetre(g, ijfl, irose) ? "oui" : "non");

    printf("\n******* les ancetres communs:\n");
    printf("%s et %s ont ancetre commun: %s\n", nomIndividu(getByIdent(g, irose)), nomIndividu(getByIdent(g, ij)), ontAncetreCommun(g, irose, ij) ? "oui" : "non");
    printf("%s et %s ont ancetre commun: %s\n", nomIndividu(getByIdent(g, ih)), nomIndividu(getByIdent(g, ibill)), ontAncetreCommun(g, ih, ibill) ? "oui" : "non");

    printf("\n******* ancetre plus ancien:\n");
    printf("L'ancetre le plus ancien de %s est %s\n", nomIndividu(getByIdent(g, ia2)), nomIndividu(getByIdent(g, plus_ancien(g, ia2))));

    printf("\n******* parente:\n");
    printf("parents de %s\n", nomIndividu(getByIdent(g, ia2)));
    buf[0] = 0; affiche_parente(g, ia2, buf);
    printf("%s\n", buf);

    printf("\n******* descendence:\n");
    printf("descendence de %s\n", nomIndividu(getByIdent(g, ijm)));
    buf[0] = 0;  affiche_descendance(g, ijm, buf);
    printf("%s\n", buf);

    printf("\n******* free:\n");
    genealogieFree(&g);
    printf("fin.(press key)\n");
    fgets(buf, 2, stdin);
    return 0;
}

