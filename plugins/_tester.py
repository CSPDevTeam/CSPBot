import traceback
import json
def fun():
      s =  traceback.extract_stack()
      print('%s Invoked me!'%s[-2][2])
      ps = s.__getitem__(-2)
      print(len(ps))
      print('%s Invoked me!'%ps.__getitem__(0))

json.loads()

fun()