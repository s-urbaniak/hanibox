#ifndef __REC_ALSA_DEVICES_H__
#define __REC_ALSA_DEVICES_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define REC_ALSA_ERROR (rec_alsa_error_quark())

typedef enum
{
  REC_ALSA_ERROR_NO_SOUNDCARDS,
} RecAlsaError;

typedef struct {

  GString *card, *device, *name;
} RecAlsaDevice;

GArray* rec_alsa_devices_new (GError **error);
GQuark rec_alsa_error_quark (void);

G_END_DECLS

#endif /* REC_ALSA_DEVICES_H */
