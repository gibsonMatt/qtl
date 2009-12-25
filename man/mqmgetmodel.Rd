\name{mqmgetmodel}
\alias{mqmgetmodel}
\alias{mqmgetmodel}

\title{ Retrieve the QTL model used in mapping from the results of an MQM scan }

\description{
  Retrieves the QTL model used for scanning from the output of an MQM scan. The
  model only contains the selected cofactors significant at the specified
  cofactor.significance from the results of an mqm scan
}

\usage{
	mqmgetmodel(scanresult)
}

\arguments{
  \item{scanresult}{
    % \scanoneobject
  } 
}

\value{
  The function returns the QTL model created (cofactors selected in modeling
  found to be significant at cofactor.significance) and used during the mqmscan
  See for the format of the model: \code{\link{makeqtl}} returns NULL when no
  cofactors reach the significance threshold during QTL modeling.
}  

\author{ 
Ritsert C Jansen; Danny Arends; Pjotr Prins; Karl W Broman \email{kbroman@biostat.wisc.edu} % \mqmauthors
}

\seealso{
	\itemize{
     \item \code{\link{mqmsetcofactors}} - Setting multiple cofactors for backward elimination
    \item \code{\link{makeqtl}} - Make a qtl object
   % \input{"inst/docs/mqm/standard_seealso.txt"}
    \item \code{\link{MQM}} - MQM description and references
    \item \code{\link{mqmscan}} - Main MQM single trait analysis
    \item \code{\link{mqmall}} - Parallellized traits analysis 
    \item \code{\link{mqmaugment}} - Augmentation routine for estimating missing data
    \item \code{\link{mqmcofactors}} - Set cofactors for the MQM algorithm
    \item \code{\link{mqmbootstrap}} - Estimate confidence intervals
    \item \code{\link{scanone}} - Single QTL scanning
% -----^^ inst/docs/mqm/standard_seealso.txt ^^-----
	}
}

\keyword{ utilities }