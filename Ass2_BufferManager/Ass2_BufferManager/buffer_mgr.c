//
//  buffer_mgr.c
//  Ass2_BufferManager
//
//  Created by Juan Luis Polo and Eduardo Moreno on Spring 2020.
//  Copyright © 2020 Juan Luis Polo and Eduardo Moreno. All rights reserved.
//

#include <stdio.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"


RC forceFlushPool(BM_BufferPool *const bm){
    
    int i = 0;
    for (i = 0; i < (*bm).numPages; i++) {
        
    }
    
    return 0;
}

