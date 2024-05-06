
void Group::OnCreate(SDL_Renderer *renderer)
{
    for (auto &e : mEntities)
    {
        e->OnCreate(renderer);
    }

    // Les objects sont créés avec un renderer et une texture, on peut donc les positionner à l'écran
    // ligne par ligne

    int y = 0;
    int x = 0;
    int x_max = 0;
    int y_max = 0;

    for (int i = 0; i < mGrid.size(); i++)
    {
        x = 0;
        y_max = 0;

        if (mGrid[i].size() == 0)
        {
            // Skip empty line
            continue;
        }

        // Première boucle pour déterminer la largeur totale de tous les objets
        // Ainsi que la hauteur max de la ligne
        for (auto &l : mGrid[i])
        {
            x += l->GetRect().w;
            if (l->GetRect().h > y_max)
            {
                y_max = l->GetRect().h;
            }

            if (x > x_max)
            {
                x_max = x;
            }
        }

        int offset = (GetSystem().GetScreenW() - x_max) / (mGrid[i].size() + 1);

        // Deuxième boucle pour placer les objets
        int start_x = offset;
        for (auto &l : mGrid[i])
        {
            l->SetPos(start_x, y);

            start_x += offset + l->GetRect().w;
        }

        // new line
        y += y_max + 10;
    }

    mGridH = y;
    mGridW = x_max;
}
