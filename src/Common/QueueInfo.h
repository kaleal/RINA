// The MIT License (MIT)
//
// Copyright (c) 2014-2016 Brno University of Technology, PRISTINE project
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
/*
 * @file QueueInfo.h
 * @author Marcel Marek
 * @date Apr 28, 2016
 * @brief
 * @detail
 */

#ifndef QUEUEINFO_H_
#define QUEUEINFO_H_

#include <omnetpp.h>
#include "Flow.h"

class QueueInfo : public cObject
{
  private:
    Flow* flow;
    unsigned int capacity;
    unsigned int free;

  public:
    QueueInfo();
    virtual ~QueueInfo();
    unsigned int getCapacity() const;
    void setCapacity(unsigned int capacity);
    Flow* getFlow() const;
    void setFlow(Flow* flow);
    unsigned int getFree() const;
    void setFree(unsigned int free);
};

#endif /* QUEUEINFO_H_ */
