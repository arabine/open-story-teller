#ifndef I_AUDIO_EVENT_H
#define I_AUDIO_EVENT_H

// Interface pour gérer les événements audio, comme la fin d'une piste.
// Ceci permet de découpler le lecteur audio de l'entité qui doit réagir à ces événements (par exemple, la VM).

class IAudioEvent
{
public:
    virtual ~IAudioEvent() = default;

    // Appelé lorsque la lecture d'une piste audio est terminée.
    virtual void EndOfAudio() = 0;
};

#endif // I_AUDIO_EVENT_H
