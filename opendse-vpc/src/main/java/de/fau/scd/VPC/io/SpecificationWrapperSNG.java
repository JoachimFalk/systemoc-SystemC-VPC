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

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Set;

import org.opt4j.core.start.Constant;

import com.google.inject.Inject;

import de.fau.scd.VPC.io.Common.FormatErrorException;
import de.fau.scd.VPC.io.SNGImporter.FIFOTranslation;

import net.sf.opendse.model.Application;
import net.sf.opendse.model.Architecture;
import net.sf.opendse.model.Dependency;
import net.sf.opendse.model.Link;
import net.sf.opendse.model.Mappings;
import net.sf.opendse.model.Resource;
import net.sf.opendse.model.Specification;
import net.sf.opendse.model.Task;
import net.sf.opendse.optimization.SpecificationWrapper;
import net.sf.opendse.optimization.io.SpecificationTransformer;
import net.sf.opendse.optimization.io.SpecificationWrapperInstance;

public class SpecificationWrapperSNG implements SpecificationWrapper {

    final private SpecificationWrapperInstance specificationWrapperInstance;

    @Inject
    public SpecificationWrapperSNG(
        @Constant(namespace = SpecificationWrapperSNG.class, value = "sngFile") String sngFileName
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "vpcConfigTemplate") String vpcConfigTemplate
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "fifoTranslation")   FIFOTranslation fifoTranslation
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "multicastMessages") boolean multicastMessages
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "shareFIFOBuffers")  boolean shareFIFOBuffers
        ) throws IOException, FileNotFoundException, FormatErrorException
    {
        UniquePool uniquePool = new UniquePool();

        SNGReader sngReader = new SNGReader(sngFileName);
        VPCConfigReader vpcConfigReader = new VPCConfigReader(vpcConfigTemplate);

        Boolean generateMulticast = null;
        switch (fifoTranslation) {
        case FIFO_IS_MESSAGE:
            generateMulticast = multicastMessages;
            break;
        case FIFO_IS_MEMORY_TASK:
            generateMulticast = shareFIFOBuffers;
            break;
        }
        assert generateMulticast != null : "Oops, internal error!";

        SNGImporter sngImporter = new SNGImporter(sngReader, uniquePool, fifoTranslation, generateMulticast);
        Application<Task, Dependency> application = sngImporter.getApplication();

        VPCConfigImporter vpcConfigImporter = new VPCConfigImporter(vpcConfigReader,uniquePool, application);
        Architecture<Resource, Link> architecture = vpcConfigImporter.getArchitecture();
        Mappings<Task, Resource> mappings = vpcConfigImporter.getMappings();

        Specification specification = new Specification(application, architecture, mappings);
        specificationWrapperInstance = new SpecificationWrapperInstance(specification);
    }

    @Override
    public Specification getSpecification() {
        return specificationWrapperInstance.getSpecification();
    }

    @Inject(optional = true)
    public void setSpecificationTransformers(Set<SpecificationTransformer> transformers) {
        specificationWrapperInstance.setSpecificationTransformers(transformers);
    }

}