#ifndef __REC_RECORDER_H__
#define __REC_RECORDER_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define REC_TYPE_RECORDER (rec_recorder_get_type())

G_DECLARE_FINAL_TYPE (RecRecorder, rec_recorder, REC, RECORDER, GObject)

  RecRecorder *
  rec_recorder_new (void);


G_END_DECLS

#endif /* REC_RECORDER_H */
