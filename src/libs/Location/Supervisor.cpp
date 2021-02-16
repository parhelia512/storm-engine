//============================================================================================
//	Spirenkov Maxim aka Sp-Max Shaman, 2001
//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
//	Supervisor
//--------------------------------------------------------------------------------------------
//
//============================================================================================

#include "Supervisor.h"
#include "Character.h"
#include "Entity.h"
#include "LocatorArray.h"
#include "core.h"

//============================================================================================
//Конструирование, деструктурирование
//============================================================================================

Supervisor::Supervisor()
{
    numCharacters = 0;
    time = 0.0f;
    waveTime = 0.0f;
    curUpdate = 0;
    player = nullptr;
}

Supervisor::~Supervisor()
{
    isDelete = true;
    for (long i = 0; i < numCharacters; i++)
    {
        character[i].c->AlreadySTORM_DELETE();
        EntityManager::EraseEntity(character[i].c->GetId());
    }
}

//Добавить персонажа в локацию
void Supervisor::AddCharacter(Character *ch)
{
    Assert(ch);
    if (numCharacters >= MAX_CHARACTERS)
        throw std::exception("Number of characters amount to criticle value, don't create new character");
    character[numCharacters].c = ch;
    character[numCharacters++].lastTime = time;
}

//Удалить персонажа из локации
void Supervisor::DelCharacter(Character *ch)
{
    for (long i = 0; i < numCharacters; i++)
        if (character[i].c == ch)
        {
            character[i] = character[--numCharacters];
            return;
        }
}

void Supervisor::Update(float dltTime)
{
    //Если нет персонажей, ничего не делаем
    if (!numCharacters)
        return;
    //Перемещаем персонажей
    for (long i = 0; i < numCharacters; i++)
    {
        character[i].c->Move(dltTime);
        character[i].c->colMove = 0.0f;
        character[i].c->isCollision = false;
    }
    //Вычисляем дистанции, и определяем взаимодействующих персонажей
    long chr = 0;
    long i;
    for (i = 0; i < numCharacters - 1; i++)
    {
        //Пропустим мёртвых
        if (character[i].c->liveValue < 0 || character[i].c->deadName)
            continue;
        character[i].c->startColCharacter = chr;
        auto curPos(character[i].c->curPos);
        const auto radius = character[i].c->radius;
        for (auto j = i + 1; j < numCharacters; j++)
        {
            //Пропустим мёртвых
            auto *ci = character[i].c;
            auto *cj = character[j].c;
            if (cj->liveValue < 0 || cj->deadName)
                continue;
            //Расстояние между персонажами
            auto d = ~(curPos - cj->curPos);
            //Растояние взаимодействия персонажей
            auto r = radius + cj->radius;
            const auto rr = r * 4.0f;
            if (d > rr * rr)
                continue;
            //Сохраняем характера
            colchr[chr].c = cj;
            colchr[chr].d = sqrtf(d);
            colchr[chr].maxD = rr;
            chr++;
            //Проверяем пересечение персонажей по высоте
            if (cj->curPos.y > ci->curPos.y + ci->height)
                continue;
            if (ci->curPos.y > cj->curPos.y + cj->height)
                continue;
            //Расталкиваем персонажей
            auto dx = curPos.x - cj->curPos.x;
            auto dz = curPos.z - cj->curPos.z;
            d = dx * dx + dz * dz;
            r *= 0.5f;
            if (d >= r * r)
                continue;
            if (d)
            {
                d = sqrtf(d);
                d = (r - d) / d;
                dx *= d;
                dz *= d;
                ci->isCollision = true;
                cj->isCollision = true;
                auto moveI = ci->IsMove();
                if ((~ci->impulse) > 0.0001f && !moveI)
                {
                    moveI = ((ci->impulse.x * dx + ci->impulse.z * dz) < 0.0f);
                }
                auto moveJ = cj->IsMove();
                if ((~cj->impulse) > 0.0001f && !moveJ)
                {
                    moveJ = ((cj->impulse.x * dx + cj->impulse.z * dz) > 0.0f);
                }
                if (ci->IsFight())
                {
                    if (moveI == moveJ)
                    {
                        ci->curPos.x += dx * 0.5f;
                        ci->curPos.z += dz * 0.5f;
                        cj->curPos.x -= dx * 0.5f;
                        cj->curPos.z -= dz * 0.5f;
                    }
                    else
                    {
                        if (moveI)
                        {
                            ci->curPos.x += dx * 0.999f;
                            ci->curPos.z += dz * 0.999f;
                            cj->curPos.x -= dx * 0.001f;
                            cj->curPos.z -= dz * 0.001f;
                        }
                        else
                        {
                            ci->curPos.x += dx * 0.001f;
                            ci->curPos.z += dz * 0.001f;
                            cj->curPos.x -= dx * 0.999f;
                            cj->curPos.z -= dz * 0.999f;
                        }
                    }
                }
                else if (moveI == moveJ)
                {
                    ci->curPos.x += dx * 0.5f;
                    ci->curPos.z += dz * 0.5f;
                    cj->curPos.x -= dx * 0.5f;
                    cj->curPos.z -= dz * 0.5f;
                }
                else
                {
                    if (moveI)
                    {
                        ci->curPos.x += dx * 0.9f;
                        ci->curPos.z += dz * 0.9f;
                        cj->curPos.x -= dx * 0.1f;
                        cj->curPos.z -= dz * 0.1f;
                    }
                    else
                    {
                        ci->curPos.x += dx * 0.1f;
                        ci->curPos.z += dz * 0.1f;
                        cj->curPos.x -= dx * 0.9f;
                        cj->curPos.z -= dz * 0.9f;
                    }
                }
            }
        }
        character[i].c->numColCharacter = chr - character[i].c->startColCharacter;
    }
    character[i].c->numColCharacter = 0;
    //Вычисления
    for (i = 0; i < numCharacters; i++)
        character[i].c->Calculate(dltTime);
    //Коллизия персонажей и установка новых координат
    for (i = 0; i < numCharacters; i++)
        character[i].c->Update(dltTime);
}

void Supervisor::PreUpdate(float dltTime) const
{
    //Сбрасываем состояние персонажей
    for (long i = 0; i < numCharacters; i++)
        character[i].c->Reset();
    core.Event("CharactersStateUpdate", "f", dltTime);
}

void Supervisor::PostUpdate(float dltTime)
{
    //Обновление состояния персонажа
    time += dltTime;
    waveTime += dltTime;
    //Корректируем время при превышении предела
    if (time > 10000.0f)
    {
        for (long i = 0; i < numCharacters; i++)
            character[i].lastTime -= time;
        time -= 10000.0f;
    }
    //Если не пришло время обновления, пропустим ход
    if (curUpdate >= numCharacters)
    {
        if (waveTime < 0.1f)
            return;
        waveTime = 0.0f;
        curUpdate = 0;
    }
    //Исполняем текущего персонажа
    if (numCharacters)
    {
        for (long i = 0; i < 5; i++)
        {
            if (curUpdate >= numCharacters)
                break;
            const auto dlt = time - character[curUpdate].lastTime;
            character[curUpdate].lastTime = time;
            if (EntityManager::GetEntityPointer(character[curUpdate].c->GetId()))
            {
                core.Event("CharacterUpdate", "if", character[curUpdate].c->GetId(), dlt);
            }
            curUpdate++;
        }
    }
}

//Установить позиции для загрузки
void Supervisor::SetSavePositions() const
{
    for (long i = 0; i < numCharacters; i++)
    {
        if (!character[i].c)
            continue;
        character[i].c->SetSavePosition();
    }
}

//Удалить позиции для загрузки
void Supervisor::DelSavePositions(bool isTeleport) const
{
    for (long i = 0; i < numCharacters; i++)
    {
        if (!character[i].c)
            continue;
        character[i].c->DelSavePosition(isTeleport);
    }
}

//Проверить на свободность позицию
bool Supervisor::CheckPosition(float x, float y, float z, Character *c) const
{
    for (long i = 0; i < numCharacters; i++)
    {
        if (character[i].c == c)
            continue;
        const auto dx = x - character[i].c->curPos.x;
        const auto dy = y - character[i].c->curPos.y;
        const auto dz = z - character[i].c->curPos.z;
        if (fabsf(dy) > character[i].c->height * 0.8f)
            continue;
        if (dx * dx + dz * dz > character[i].c->radius * 0.8f)
            continue;
        return false;
    }
    return true;
}

//Найти по радиусу персонажей
bool Supervisor::FindCharacters(FindCharacter fndCharacter[MAX_CHARACTERS], long &numFndCharacters, Character *chr,
                                float radius, float angTest, float nearPlane, float ax, bool isSort,
                                bool lookCenter) const
{
    numFndCharacters = 0;
    if (!chr || radius < 0.0f)
        return false;
    //Радиус тестирования
    radius *= radius;
    //Позиция персонажа
    auto x = chr->curPos.x;
    auto y = chr->curPos.y;
    auto z = chr->curPos.z;
    //Параметры для тестирования в секторе на x_z
    CVECTOR N1, N2, N3;
    float d1, d2, d3;
    if (angTest > 0.0f)
    {
        CMatrix m(0.0f, chr->ay, 0.0f);
        auto ang = 0.5f * angTest * 3.141592654f / 180.0f;
        N1 = m * CVECTOR(cosf(ang), 0.0f, sinf(ang));
        d1 = N1 | chr->curPos;
        N2 = m * CVECTOR(-cosf(-ang), 0.0f, -sinf(-ang));
        d2 = N2 | chr->curPos;
        N3 = m.Vz();
        d3 = N3 | chr->curPos;
    }
    //Параметры для тестирования в секторе по y_dist
    if (ax > 0.0f)
    {
        ax = sinf(0.5f * ax * 3.141592654f / 180.0f);
    }
    else
        ax = 1.0f;
    if (ax < 0.0f)
        ax = 0.0f;
    if (ax > 1.0f)
        ax = 1.0f;
    ax *= ax;
    auto testY = y + chr->height * 0.5f;
    //Просматриваем персонажей
    for (long i = 0; i < numCharacters; i++)
    {
        //Исключим себя
        if (character[i].c == chr)
            continue;
        //Проверка на убитых
        if (character[i].c->liveValue < 0 || character[i].c->deadName)
            continue;
        //По дистанции
        auto dx = character[i].c->curPos.x - x;
        auto dz = character[i].c->curPos.z - z;
        auto d = dx * dx + dz * dz;
        if (d > radius)
            continue;
        //По высоте
        auto dy = character[i].c->curPos.y + character[i].c->height - testY;
        if (dy < 0.0f && dy * dy > d * ax)
            continue;
        dy = testY - character[i].c->curPos.y;
        if (dy < 0.0f && dy * dy > d * ax)
            continue;
        //В плоскости xz
        auto dist1 = 0.0f;
        auto dist2 = 0.0f;
        auto dist3 = 0.0f;
        if (angTest > 0.0f && d > 1.0f) // eddy. при близком подходе со спины помещать в структуру
        {
            //Проверим расположение
            auto rad = !lookCenter ? -character[i].c->radius : 0.0f;
            dist1 = (N1 | character[i].c->curPos) - d1;
            if (dist1 < rad)
                continue;
            dist2 = (N2 | character[i].c->curPos) - d2;
            if (dist2 < rad)
                continue;
            dist3 = (N3 | character[i].c->curPos) - d3;
            if (dist3 < nearPlane)
                continue;
        }
        //Добавляем
        fndCharacter[numFndCharacters].c = character[i].c;
        fndCharacter[numFndCharacters].dx = dx;
        fndCharacter[numFndCharacters].dy = dy;
        fndCharacter[numFndCharacters].dz = dz;
        fndCharacter[numFndCharacters].d2 = d;
        numFndCharacters++;
    }
    if (isSort)
    {
        for (long i = 0; i < numFndCharacters - 1; i++)
        {
            auto k = i;
            for (auto j = i + 1; j < numFndCharacters; j++)
            {
                if (fndCharacter[k].d2 > fndCharacter[j].d2)
                    k = j;
            }
            if (k != i)
            {
                auto fc = fndCharacter[i];
                fndCharacter[i] = fndCharacter[k];
                fndCharacter[k] = fc;
            }
        }
    }
    return numFndCharacters != 0;
}

//Найти оптимальный локатор для продолжения прогулки персонажа
long Supervisor::FindForvardLocator(LocatorArray *la, const CVECTOR &pos, const CVECTOR &norm, bool lookChr) const
{
    if (!la)
        return -1;
    const auto num = la->Num();
    CVECTOR lpos;
    float maxcs;
    long l = -1;
    for (long i = 0; i < num; i++)
    {
        if (!la->GetLocatorPos(i, lpos.x, lpos.y, lpos.z))
            continue;
        if (lookChr)
        {
            if (!CheckPosition(lpos.x, lpos.y, lpos.z, nullptr))
                continue;
        }
        lpos -= pos;
        lpos.y = 0.0f;
        auto cs = lpos.x * lpos.x + lpos.z * lpos.z;
        if (cs <= 0.0f)
            continue;
        lpos *= 1.0f / sqrtf(cs);
        cs = lpos | norm;
        if (l > 0)
        {
            if (cs > maxcs)
            {
                l = i;
                maxcs = cs;
            }
        }
        else
        {
            l = i;
            maxcs = cs;
        }
    }
    if (l >= 0 && la->GetLocatorPos(l, lpos.x, lpos.y, lpos.z))
    {
        if (!CheckPosition(lpos.x, lpos.y, lpos.z, nullptr))
        {
            return FindForvardLocator(la, pos, norm, true);
        }
    }
    return l;
}