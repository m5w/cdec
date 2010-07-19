#ifndef SCORER_H_
#define SCORER_H_
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
//TODO: use intrusive shared_ptr in Score (because there are many of them on ErrorSurfaces)
#include "wordid.h"

class Score;
class SentenceScorer;
typedef boost::shared_ptr<Score> ScoreP;
typedef boost::shared_ptr<SentenceScorer> ScorerP;

class ViterbiEnvelope;
class ErrorSurface;
class Hypergraph;  // needed for alignment

enum ScoreType { IBM_BLEU, NIST_BLEU, Koehn_BLEU, TER, BLEU_minus_TER_over_2, SER, AER, IBM_BLEU_3 };
ScoreType ScoreTypeFromString(const std::string& st);
std::string StringFromScoreType(ScoreType st);

class Score {
 public:
  virtual ~Score();
  virtual float ComputeScore() const = 0;
  virtual float ComputePartialScore() const =0;
  virtual void ScoreDetails(std::string* details) const = 0;
  std::string ScoreDetails() {
    std::string d;
    ScoreDetails(&d);
    return d;
  }
  virtual void PlusEquals(const Score& rhs, const float scale) = 0;
  virtual void PlusEquals(const Score& rhs) = 0;
  virtual void PlusPartialEquals(const Score& rhs, int oracle_e_cover, int oracle_f_cover, int src_len) = 0;
  virtual void Subtract(const Score& rhs, Score *res) const = 0;
  virtual ScoreP GetZero() const = 0;
  virtual ScoreP GetOne() const = 0;
  virtual bool IsAdditiveIdentity() const = 0; // returns true if adding this delta
                                      // to another score results in no score change
				      // under any circumstances
  virtual void Encode(std::string* out) const = 0;
  static ScoreP GetZero(ScoreType type);
  static ScoreP GetOne(ScoreType type);
};

class SentenceScorer {
 public:
  typedef std::vector<WordID> Sentence;
  typedef std::vector<Sentence> Sentences;
  std::string desc;
  Sentences refs;
  SentenceScorer(std::string desc="SentenceScorer_unknown", Sentences const& refs=Sentences()) : desc(desc),refs(refs) {  }
  std::string verbose_desc() const;
  virtual float ComputeRefLength(const Sentence& hyp) const; // default: avg of refs.length
  virtual ~SentenceScorer();
  virtual ScoreP GetOne() const;
  virtual ScoreP GetZero() const;
  void ComputeErrorSurface(const ViterbiEnvelope& ve, ErrorSurface* es, const ScoreType type, const Hypergraph& hg) const;
  virtual ScoreP ScoreCandidate(const Sentence& hyp) const = 0;
  virtual ScoreP ScoreCCandidate(const Sentence& hyp) const =0;
  virtual const std::string* GetSource() const;
  static ScoreP CreateScoreFromString(const ScoreType type, const std::string& in);
  static ScorerP CreateSentenceScorer(const ScoreType type,
    const std::vector<Sentence >& refs,
    const std::string& src = "");
};

//TODO: should be able to GetOne GetZero without supplying sentence (just type)
class DocScorer {
 public:
  ~DocScorer();
  DocScorer() {  }
  void Init(const ScoreType type,
            const std::vector<std::string>& ref_files,
            const std::string& src_file = "",
            bool verbose=false
    );
  DocScorer(const ScoreType type,
            const std::vector<std::string>& ref_files,
            const std::string& src_file = "",
            bool verbose=false
    )
  {
    Init(type,ref_files,src_file,verbose);
  }

  int size() const { return scorers_.size(); }
  ScorerP operator[](size_t i) const { return scorers_[i]; }
 private:
  std::vector<ScorerP> scorers_;
};


#endif
