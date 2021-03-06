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

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.Set;
import java.util.TreeSet;

import org.opt4j.core.start.Constant;

import com.google.inject.Inject;

import de.fau.scd.VPC.helper.TempDirectoryHandler;
import de.fau.scd.VPC.io.Common.FormatErrorException;
import de.fau.scd.VPC.io.SNGImporter.ChanTranslation;
import de.fau.scd.VPC.io.SNGImporter.FIFOMerging;

import net.sf.opendse.model.Application;
import net.sf.opendse.model.Architecture;
import net.sf.opendse.model.Dependency;
import net.sf.opendse.model.Link;
import net.sf.opendse.model.Mappings;
import net.sf.opendse.model.Resource;
import net.sf.opendse.model.Specification;
import net.sf.opendse.model.Task;
import net.sf.opendse.optimization.SpecificationWrapper;
import net.sf.opendse.optimization.encoding.RoutingFilter;
import net.sf.opendse.optimization.io.SpecificationTransformer;
//import net.sf.opendse.optimization.io.SpecificationWrapperInstance;

public class SpecificationWrapperSNG implements SpecificationWrapper {
    
    public enum DFGSource {
        DFG_FROM_SNG_FILE
      , DFG_FROM_SIM_EXPORT
    };
    
    public interface SimulatorEnvironment {
        public String [] getEnvironment();
    }

    public interface SimulatorArguments {
        public String [] getArguments();
    }

    @Inject
    public SpecificationWrapperSNG(
        @Constant(namespace = SpecificationWrapperSNG.class, value = "dfgSource")
        DFGSource            dfgSource
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "sngFile")
        String               sngFileName
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "simulatorExecutable")
        String               simulatorExecutable
      , SimulatorArguments   simulatorArguments
      , SimulatorEnvironment simulatorEnvironment
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "vpcConfigTemplate")
        String               vpcConfigTemplate
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "fillInRoutings")
        boolean              fillInRoutings
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "chanTranslation")
        ChanTranslation      chanTranslation
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "fifoMerging")
        FIFOMerging          fifoMerging
      , @Constant(namespace = SpecificationWrapperSNG.class, value = "multicastMessages")
        boolean              multicastMessages
        ) throws IOException, FileNotFoundException, FormatErrorException
    {
        UniquePool uniquePool = new UniquePool();

        SNGReader sngReader = null;
        switch (dfgSource) {
        case DFG_FROM_SNG_FILE:
            sngReader = new SNGReader(sngFileName);
            break;
        case DFG_FROM_SIM_EXPORT:
            TempDirectoryHandler tempDirectoryHandler =
                TempDirectoryHandler.getTempDirectoryHandler();
            File sngFile = Files.createTempFile(
                    tempDirectoryHandler.getDirectory().toPath()
                  , "dfg", ".sng").toFile();
            
            try {
                ArrayList<String> cmd = new ArrayList<String>();
                cmd.add(simulatorExecutable);
                cmd.add("--systemoc-export-sng");
                cmd.add(sngFile.getAbsolutePath());
                for (String arg : simulatorArguments.getArguments()) {
                    cmd.add(arg);
                }
                System.out.println(cmd.toString());
                Process exec_process = Runtime.getRuntime().exec(
                    cmd.toArray(new String[0]), simulatorEnvironment.getEnvironment(),
                    tempDirectoryHandler.getDirectory());
                int status = exec_process.waitFor();
                if (status != 0) {
                    sngFile.delete();
                    sngFile = null;
                }
            } catch (IOException e) {
                System.err.println("Got an exception during DFG export from simulator:\n" + e);
                sngFile.delete();
                sngFile = null;
            } catch (InterruptedException e) {
                System.err.println("Got an exception during DFG export from simulator:\n" + e);
                sngFile.delete();
                sngFile = null;
            }            
            sngReader = new SNGReader(sngFile);
            break;
        }

        SNGImporter sngImporter = new SNGImporter(sngReader, uniquePool, chanTranslation, fifoMerging, multicastMessages);
        Application<Task, Dependency> application = sngImporter.getApplication();

        VPCConfigReader vpcConfigReader = new VPCConfigReader(vpcConfigTemplate);
        VPCConfigImporter vpcConfigImporter = new VPCConfigImporter(vpcConfigReader,uniquePool, application);
        Architecture<Resource, Link> architecture = vpcConfigImporter.getArchitecture();
        Mappings<Task, Resource> mappings = vpcConfigImporter.getMappings();

        if (fillInRoutings)
            this.specification = new Specification(application, architecture, mappings);
        else
            this.specification = new Specification(application, architecture, mappings, null);
    }

    protected final Specification specification;
    protected boolean init = false;

    protected final  Set<SpecificationTransformer> transformers = new TreeSet<SpecificationTransformer>(
        new Comparator<SpecificationTransformer>() {
            @Override
            public int compare(SpecificationTransformer o1, SpecificationTransformer o2) {
                return ((Integer) o1.getPriority()).compareTo(o2.getPriority());
            }
        });

    @Override
    public Specification getSpecification() {
        if (!init) {
            init = true;
            if (transformers != null) {
                for (SpecificationTransformer specificationTransformer : transformers) {
                    System.out.println("Starting " + specificationTransformer);
                    specificationTransformer.transform(specification);
                }
            }
            if (specification.getRoutings() != null)
                RoutingFilter.filter(this.specification);
        }
        return specification;
    }

    @Inject(optional = true)
    public void setSpecificationTransformers(Set<SpecificationTransformer> transformers) {
        this.transformers.addAll(transformers);
    }

}