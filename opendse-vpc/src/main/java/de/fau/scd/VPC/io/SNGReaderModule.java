// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */
package de.fau.scd.VPC.io;

import de.fau.scd.VPC.io.SNGImporter.FIFOTranslation;

import net.sf.opendse.optimization.SpecificationWrapper;
import net.sf.opendse.optimization.io.IOModule;

import org.opt4j.core.config.annotations.File;
import org.opt4j.core.config.annotations.Info;
import org.opt4j.core.config.annotations.Order;
import org.opt4j.core.config.annotations.Required;
import org.opt4j.core.start.Constant;

public class SNGReaderModule extends IOModule {

    @Info("The dataflow graph given as SNG XML file.")
    @Order(0)
    @Constant(namespace = SpecificationWrapperSNG.class, value = "sngFile")
    @File
    protected String sngFile = "";

    public String getSngFile() {
        return sngFile;
    }

    public void setSngFile(String sngFile) {
        this.sngFile = sngFile;
    }

    @Info("The architecture given as VPC configuration XML template file.")
    @Order(1)
    @Constant(namespace = SpecificationWrapperSNG.class, value = "vpcConfigTemplate")
    @File
    protected String vpcConfigTemplate = "";

    public String getVpcConfigTemplate() {
        return vpcConfigTemplate;
    }

    public void setVpcConfigTemplate(String vpcConfigTemplate) {
        this.vpcConfigTemplate = vpcConfigTemplate;
    }

    @Info("Select how FIFOs are translated into the DSE model.")
    @Order(3)
    @Constant(value = "fifoTranslation", namespace = SpecificationWrapperSNG.class)
    protected FIFOTranslation fifoTranslation = FIFOTranslation.FIFO_IS_MEMORY_TASK;

    public FIFOTranslation getFifoTranslation() {
        return fifoTranslation;
    }

    public void setFifoTranslation(FIFOTranslation fifoTranslation) {
        this.fifoTranslation = fifoTranslation;
    }

    @Info("If true, multicast communication is generated for FIFOs into which identical data is written.")
    @Order(2)
    @Constant(namespace = SpecificationWrapperSNG.class, value = "multicastMessages")
    @Required(property = "fifoTranslation", elements = { "FIFO_IS_MESSAGE" })
    protected boolean multicastMessages = true;

    public boolean getMulticastMessages() {
        return multicastMessages;
    }

    public void setMulticastMessages(boolean multicastMessages) {
        this.multicastMessages = multicastMessages;
    }

    @Info("If true, only one memory task is generated for FIFOs into which identical data is written.")
    @Order(2)
    @Constant(namespace = SpecificationWrapperSNG.class, value = "shareFIFOBuffers")
    @Required(property = "fifoTranslation", elements = { "FIFO_IS_MEMORY_TASK" })
    protected boolean shareFIFOBuffers = true;

    public boolean getShareFIFOBuffers() {
        return shareFIFOBuffers;
    }

    public void setShareFIFOBuffers(boolean shareFIFOBuffers) {
        this.shareFIFOBuffers = shareFIFOBuffers;
    }

    @Override
    protected void config() {
        bind(SpecificationWrapper.class).to(SpecificationWrapperSNG.class).in(SINGLETON);
    }

}