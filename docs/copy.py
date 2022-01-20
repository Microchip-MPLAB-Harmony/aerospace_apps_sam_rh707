#!/usr/bin/env python

import os, re

items = [["GUID-FB6D209F-8F8E-489B-81C7-F2F51AA825EB.md", "icm_with_ecc_error_injection"],
         ["GUID-8F37E0B5-6CF5-46A7-AD5E-27A373C8B7BC.md", os.path.join("ip1553","ip1553_bc_operation_blocking")],
         ["GUID-32DFF908-0626-470E-9418-AA32CF4EF83B.md", os.path.join("ip1553","ip1553_bc_operation_interrupt")],
         ["GUID-E229A96E-294F-4576-82E1-FFE498079816.md", os.path.join("ip1553","ip1553_rt_operation_blocking")],
         ["GUID-A8D3CB3C-B62B-4965-AB1E-28EB363D6E1E.md", os.path.join("ip1553","ip1553_rt_operation_interrupt")],
         ["GUID-42209B7E-3C95-4A82-AB9B-088294371FE1.md", os.path.join("spw", "spw_escChar_tch_loopback")],
         ["GUID-41376285-E24B-4130-AE60-3834589D8BFB.md", os.path.join("spw", "spw_loopback")],
         ["GUID-2D438E0A-A3BA-4FBC-BBBC-7EC07CB4D7DE.md", os.path.join("spw", "spw_rmap_loopback")]]

pattern = re.compile(r"GUID.*.png",re.I) 

for item in items:
    with open(item[0], 'r') as f:
        file = f.read()

    imagePathList = re.findall(pattern,file)
    for imagePath in imagePathList:
        file = file.replace(imagePath, os.path.join("../../docs/", imagePath))

    with open(os.path.join("..", "apps", item[1], "readme.md"), 'w') as f:
        f.write(file)
